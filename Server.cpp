#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <WinSock2.h>
#include <vector>
#include <thread>
#include <Ws2tcpip.h>
#include <format>

const std::string SERVER_IP = "192.168.56.1";
std::vector<SOCKET> CLIENTS;

int	ft_isalpha(int a)

{
	if (a >= 65 && a <= 90)
	{
		return (1);
	}
	if (a >= 97 && a <= 122)
	{
		return (1);
	}
	return (0);
}


void cliente(SOCKET client_sock)
{
    int index = 0;
    sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);
    getpeername(client_sock, (sockaddr*)&client_addr, &client_addr_len);

    char client_ip[50];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, 50);

    std::cout << "\nCliente conectado en " << client_ip << ":" << ntohs(client_addr.sin_port) << std::endl;

    char *username = (char *)calloc(50, sizeof(char));
    recv(client_sock, username, 50, 0);
    while (1)
    {
        char *buffer = (char *)calloc(1024, sizeof(char));
        recv(client_sock, buffer, 1024, 0);
        while (buffer[index])
            index++;
        char *msg = (char *)calloc(index + 1, sizeof(char));
        memcpy(msg, buffer, index);
        msg = (char *)std::format("{}: {}", username, buffer).c_str();
        for(unsigned int i = 0; i < CLIENTS.size(); i++)
        {   
            send(CLIENTS[i], msg, strlen(msg), 0);
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
    server.sin_addr.s_addr = inet_addr(SERVER_IP.c_str());
    server.sin_port = htons(5050);
    if (bind(listen_sock, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("bind failed: %d\n", WSAGetLastError());
        closesocket(listen_sock);
        WSACleanup();
        return 1;
    }
    printf("Escuchando conexiones en 192.168.56.1:5050");
    while (true)
    {
        listen(listen_sock, SOMAXCONN);
        client_size = sizeof(struct sockaddr_in);
        client_sock = accept(listen_sock, (struct sockaddr *)&server, &client_size);
        CLIENTS.push_back(client_sock);
        std::thread cliente_th(cliente, client_sock);
        cliente_th.detach();
    }

    WSACleanup();
    return 0;
}


