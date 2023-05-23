#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <WinSock2.h>
#include <vector>
#include <thread>
#include <Ws2tcpip.h>
#include <format>

const std::string SERVER_IP = "0.0.0.0";
std::vector<SOCKET> CLIENTS; //TODO: class clientes
std::vector<char *> USERNAMES;

char	*ft_strjoin(char const *s1, char const *s2)

{
	char	*ret;
	int		n;

	n = -1;
	if (*s1 == '\0' && *s2 == '\0')
		return (_strdup(""));
	ret = (char *)calloc(strlen(s1) + strlen(s2) + 1, sizeof(char));
	if (!ret)
		return (0);
	while (*s1 != '\0')
	{
		n++;
		ret[n] = *s1;
		s1++;
	}
	while (*s2 != '\0')
	{
		n++;
		ret[n] = *s2;
		s2++;
	}
	return (ret);
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
    USERNAMES.push_back(username);
    while (1)
    {
        char *buffer = (char *)calloc(1024, sizeof(char));
        int read = recv(client_sock, buffer, 1024, 0);
        if (read == -1)
        {
            break;    
        }  
        while (buffer[index])
            index++;
        char *msg = (char *)calloc(index + 1, sizeof(char));
        memcpy(msg, buffer, index);
        if (strcmp(buffer, "/exit") == 0) 
        {
            msg = (char *)std::format("{} se desconecto", username).c_str();
            for(unsigned int i = 0; i < CLIENTS.size(); i++)
            {   
                send(CLIENTS[i], msg, strlen(msg), 0);
            }
            free(buffer);
            break;
        }
        if (strcmp(buffer, "/users") == 0)
        {
            char *msg_user = USERNAMES[0];
            for(unsigned int i = 1; i < USERNAMES.size(); i++)
            {
                msg_user = ft_strjoin(msg_user, ", ");
                msg_user = ft_strjoin(msg_user, USERNAMES[i]);
            }
            send(client_sock, msg_user, strlen(msg_user), 0);
            continue;
        }
        msg = (char *)std::format("{}: {}\n", username, buffer).c_str();
        for(unsigned int i = 0; i < CLIENTS.size(); i++)
        {   
            send(CLIENTS[i], msg, strlen(msg), 0);
        }
        free(buffer);
    }
    closesocket(client_sock);
    for(unsigned int i = 0; i < CLIENTS.size(); i++)
    {
        if (CLIENTS[i] == client_sock)
        {
            CLIENTS.erase(CLIENTS.begin() + i);
            break;
        }
    }
    for(unsigned int i = 0; i < USERNAMES.size(); i++)
    {
        if (USERNAMES[i] == username)
            {
                USERNAMES.erase(USERNAMES.begin() + i);
                break;
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
    printf("Escuchando conexiones en localhost:5050");
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


