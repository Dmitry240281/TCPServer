#pragma once
#include <iostream>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <vector>
#include <mutex>
#include <thread>
//#pragma comment (lib, "Ws2_32.lib") в VS настраивается в свойствах проекта Компоновка\Ввод\Дополнительные возможности 
using std::cout;
using std::endl;

std::mutex cout_mutex;

class TcpServer {
private:
	WSAData _wsa_data;
	SOCKET server_socket;
	sockaddr_in _server_addr_{};
	int _port_ = 12345;
	char* _buffer_ = new char[1024];
public:
	TcpServer()
	{
		int err = WSAStartup(MAKEWORD(2, 2), &_wsa_data);
		if (err != 0)
		{
			std::cerr << "Ошибка инициализации WinSock." << std::endl;
		}
		server_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (server_socket == INVALID_SOCKET)
		{
			std::cerr << "Ошибка при создании сокета." << std::endl;
			WSACleanup();
		}
		_server_addr_.sin_family = AF_INET;
		_server_addr_.sin_addr.s_addr = INADDR_ANY;
		_server_addr_.sin_port = htons(_port_);
		int err_s = bind(server_socket, (sockaddr*)&_server_addr_, sizeof(_server_addr_));
		if (err_s == SOCKET_ERROR)
		{
			cout << "Сокет не создан" << endl;
			closesocket(server_socket);
			WSACleanup();
		}
		// Начало прослушивание порта
		if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR)
		{
			std::cerr << "Ошибка в начале прослушивания." << std::endl;
			closesocket(server_socket);
			WSACleanup();
		}
	}

	~TcpServer() 
	{
		closesocket(server_socket);
		WSACleanup();
	}

	void HandleClient(SOCKET client_socket, int client_number)
	{
		{
			std::lock_guard<std::mutex> lock(cout_mutex);
			std::cout << "Клиент №" << client_number << " подключился." << std::endl;
		}

		//char buffer[1024];
		int bytes_received;

		// Получение данных от клиента
		bytes_received = recv(client_socket, _buffer_, sizeof(_buffer_), 0);
		if (bytes_received > 0)
		{
			_buffer_[bytes_received] = '\0';  // Добавляем завершающий 0

			{
				std::lock_guard<std::mutex> lock(cout_mutex);
				std::cout << "Получено от клиента №" << client_number << ": " << _buffer_ << std::endl;
			}

			// Отправка ответа клинту
			std::string answer = "Данные получены: " + std::string(_buffer_);
			send(client_socket, answer.c_str(), answer.size(), 0);
		}
		else if (bytes_received == 0)
		{
			std::lock_guard<std::mutex> lock(cout_mutex);
			std::cout << "Клиент №" << client_number << " ожидает." << std::endl;
		}
		else
		{
			std::lock_guard<std::mutex> lock(cout_mutex);
			std::cerr << "Ошибка получения данных от клиента №" << client_number << std::endl;
		}

		// Закрытие сокета
		closesocket(client_socket);
		//WSACleanup();
		{
			std::lock_guard<std::mutex> lock(cout_mutex);
			std::cout << "Клиент №" << client_number << " закрыл соеденение." << std::endl;
		}
	}

	void RecieveAndAnswer() 
	{
		sockaddr_in client_addr;
		std::vector<std::thread> client_threads;
		int client_counter = 0;

		while (true) 
		{
			cout << "Сервер запущен. Ждем данные от клиента..." << endl;
			int client_addr_size = sizeof(client_addr);
			SOCKET client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_addr_size);
			if (client_socket == INVALID_SOCKET)
			{
				std::cerr << "Ошибка при подключении." << std::endl;
				continue;
			}

			// Поток для обработки клиента
			client_threads.emplace_back(std::thread(&TcpServer::HandleClient, *this, client_socket, ++client_counter));
			client_threads.back().detach();
			{
				std::lock_guard<std::mutex> lock(cout_mutex);
				std::cout << "Создан поток для клиента №" << client_counter << std::endl;
			}
			
		}
	}
};