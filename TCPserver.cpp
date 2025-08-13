// TCPserver
//

#include "TCPserver.h" 

int main()
{
    setlocale(LC_ALL, "rus");
    TcpServer server;
    server.RecieveAndAnswer();
    system("Pause");

    return 0;
}