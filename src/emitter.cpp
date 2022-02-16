#include "emitter.hpp"

Emitter::Emitter(char *address, int port)
{
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(address);
    serverAddress.sin_port = htons((u_short)port);
}

void Emitter::Begin()
{
    WSADATA wsaData;
    WSAStartup(0x0202, &wsaData);

    serverSocket = socket(PF_INET, SOCK_DGRAM, 0);
}

void Emitter::End()
{
    closesocket(serverSocket);
    WSACleanup();
}

void Emitter::Emit(char r, char g, char b)
{
    char data[] = {0x40, r, g, b};
    sendto(serverSocket, data, 4, 0, (LPSOCKADDR)&serverAddress, sizeof(sockaddr));
}

char *Emitter::GetAddress()
{
    return inet_ntoa(serverAddress.sin_addr);
}

int Emitter::GetPort()
{
    return serverAddress.sin_port;
}