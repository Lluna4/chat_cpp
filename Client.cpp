#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <string>
#include <WinSock2.h>
#include <thread>
#include <chrono>

const std::string SERVER_IP = "192.168.56.1";
const int SERVER_PORT = 5050;
const int MAX_USERNAME_LENGTH = 50;

struct msg
{
    char* username;
    char* message;
    msg() : username(nullptr), message(nullptr) {}
    ~msg() { delete[] username; delete[] message; }
};


void receve(SOCKET client)
{
    while(true)
    {
        char *msg = (char *)calloc(1024, sizeof(char));
        recv(client, msg, 1024, 0);
        if (msg[0] != ' ' && msg[0] != NULL)
            std::cout << msg << std::endl;
        free(msg);
    }
}

void ft_send(SOCKET client)
{
    char *msg;

    msg = (char *)calloc(1024, sizeof(char));

    while (true)
    {
        std::cin >> msg;
        send(client, msg, 1024, 0);
    }
}

int main(void)
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup failed: " << WSAGetLastError() << std::endl;
        return 1;
    }
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
        return 1;
    }
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(SERVER_IP.c_str());
    server_address.sin_port = htons(SERVER_PORT);
    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR)
    {
        std::cerr << "Connect failed: " << WSAGetLastError() << std::endl;
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }
    std::cout << "Cual es tu nombre de usuario? (max " << MAX_USERNAME_LENGTH << " characters): ";
    char username[50];
    std::cin >> username;
    send(client_socket, username, 50, 0);
    std::thread recieve_th(receve, client_socket);
    recieve_th.detach();
    char msg[1024];
    while (true)
    {
        std::cin.getline(msg, 1024);
        send(client_socket, msg, 1024, 0);
    }
}


