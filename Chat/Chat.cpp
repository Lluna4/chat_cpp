#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <WinSock2.h>
#include <vector>
#include <format>
#include <thread>

std::vector<SOCKET> CLIENTS;
void cliente(SOCKET client_sock)
{
    char message[1024];

    while (true)
    {
        recv(client_sock, message, sizeof(message), 0);
        for(unsigned int i = 0; i < CLIENTS.size(); i++)
        {
            if (CLIENTS[i] == client_sock)
                send(client_sock, std::format("Tu: {}", message), sizeof(message), 0);
            else
                send(client_sock, message, sizeof(message), 0);
        }
    }
}
int main()
{
    WSADATA wsa;
    SOCKET listen_sock, client_sock;
    struct sockaddr_in server;


    int recv_size, client_size;
    
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("WSAStartup failed: %d\n", WSAGetLastError());
        return 1;
    }
    listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_sock == INVALID_SOCKET) {
        printf("socket failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);
    if (bind(listen_sock, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("bind failed: %d\n", WSAGetLastError());
        closesocket(listen_sock);
        WSACleanup();
        return 1;
    }
    printf("Escuchando conexiones en 127.0.0.0:8888");
    while (true)
    {
        listen(listen_sock, SOMAXCONN);
        CLIENTS.push_back(accept(listen_sock, (struct sockaddr *)&server, (int *)sizeof(server)));
        std::thread cliente_th(cliente, client_sock);
    }
}

