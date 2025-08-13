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
//#pragma comment (lib, "Ws2_32.lib") � VS ������������� � ��������� ������� ����������\����\�������������� ����������� 
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
			std::cerr << "������ ������������� WinSock." << std::endl;
		}
		server_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (server_socket == INVALID_SOCKET)
		{
			std::cerr << "������ ��� �������� ������." << std::endl;
			WSACleanup();
		}
		_server_addr_.sin_family = AF_INET;
		_server_addr_.sin_addr.s_addr = INADDR_ANY;
		_server_addr_.sin_port = htons(_port_);
		int err_s = bind(server_socket, (sockaddr*)&_server_addr_, sizeof(_server_addr_));
		if (err_s == SOCKET_ERROR)
		{
			cout << "����� �� ������" << endl;
			closesocket(server_socket);
			WSACleanup();
		}
		// ������ ������������� �����
		if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR)
		{
			std::cerr << "������ � ������ �������������." << std::endl;
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
			std::cout << "������ �" << client_number << " �����������." << std::endl;
		}

		//char buffer[1024];
		int bytes_received;

		// ��������� ������ �� �������
		bytes_received = recv(client_socket, _buffer_, sizeof(_buffer_), 0);
		if (bytes_received > 0)
		{
			_buffer_[bytes_received] = '\0';  // ��������� ����������� 0

			{
				std::lock_guard<std::mutex> lock(cout_mutex);
				std::cout << "�������� �� ������� �" << client_number << ": " << _buffer_ << std::endl;
			}

			// �������� ������ ������
			std::string answer = "������ ��������: " + std::string(_buffer_);
			send(client_socket, answer.c_str(), answer.size(), 0);
		}
		else if (bytes_received == 0)
		{
			std::lock_guard<std::mutex> lock(cout_mutex);
			std::cout << "������ �" << client_number << " �������." << std::endl;
		}
		else
		{
			std::lock_guard<std::mutex> lock(cout_mutex);
			std::cerr << "������ ��������� ������ �� ������� �" << client_number << std::endl;
		}

		// �������� ������
		closesocket(client_socket);
		//WSACleanup();
		{
			std::lock_guard<std::mutex> lock(cout_mutex);
			std::cout << "������ �" << client_number << " ������ ����������." << std::endl;
		}
	}

	void RecieveAndAnswer() 
	{
		sockaddr_in client_addr;
		std::vector<std::thread> client_threads;
		int client_counter = 0;

		while (true) 
		{
			cout << "������ �������. ���� ������ �� �������..." << endl;
			int client_addr_size = sizeof(client_addr);
			SOCKET client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_addr_size);
			if (client_socket == INVALID_SOCKET)
			{
				std::cerr << "������ ��� �����������." << std::endl;
				continue;
			}

			// ����� ��� ��������� �������
			client_threads.emplace_back(std::thread(&TcpServer::HandleClient, *this, client_socket, ++client_counter));
			client_threads.back().detach();
			{
				std::lock_guard<std::mutex> lock(cout_mutex);
				std::cout << "������ ����� ��� ������� �" << client_counter << std::endl;
			}
			
		}
	}
};