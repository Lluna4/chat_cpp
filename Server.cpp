#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <WinSock2.h>
#include <vector>
#include <thread>
#include <ctime>
#include <chrono>
#include <Ws2tcpip.h>
#include <format>
#include <fstream>
#include <filesystem>
#include "encoding.hpp"

const std::string SERVER_IP = "0.0.0.0";
const char* key = generate_key();
int PORT = 5050;
bool UNSENT_MESSAGES = false;

class Cliente
{
public:
    Cliente() {}

    Cliente(SOCKET socket, char* username)
        :socket_(socket), username_(username)
    {}

    SOCKET get_socket()
    {
        return socket_;
    }

    char* get_username()
    {
        return username_;
    }
private:
    SOCKET socket_;
    char* username_;
};

class Message
{
public:
    Message() {}

    Message(char* username, std::string message, bool sent)
        :username_(username), message_(message), sent_(sent)
    {}

    char* get_username()
    {
        return username_;
    }

    void set_username(char* new_username)
    {
        username_ = _strdup(new_username);
    }

    std::string get_message()
    {
        return message_;
    }

    bool get_sent()
    {
        return sent_;
    }

    void sent()
    {
        sent_ = true;
    }

private:
    char* username_;
    std::string message_;
    bool sent_;
};


std::vector<Cliente> CLIENTS;
std::vector<Message> MESSAGES;

void file_sv()
{
    SOCKET listen_sock, client_sock;
    struct sockaddr_in server;
    int recv_size, client_size;
    listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_sock == INVALID_SOCKET) {
        printf("socket failed: %d\n", WSAGetLastError());
        WSACleanup();
        return ;
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(SERVER_IP.c_str());
    server.sin_port = htons(5051);
    if (bind(listen_sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("bind failed: %d\n", WSAGetLastError());
        closesocket(listen_sock);
        WSACleanup();
        return ;
    }
    printf("Servidor de archivos en localhost:5051\n");
    while (true)
    {
        listen(listen_sock, SOMAXCONN);
        client_size = sizeof(struct sockaddr_in);
        client_sock = accept(listen_sock, (struct sockaddr*)&server, &client_size);
    }
}

void chat(SOCKET cliente)
{
    send(cliente, key, strlen(key), 0);
    char* username = (char*)calloc(50, sizeof(char));
    if (!username)
    {
        return;
    }
    recv(cliente, username, 50, 0);
    username = _strdup(decode_text(username, key));
    CLIENTS.push_back(Cliente(cliente, username));
    char* mssg = (char*)std::format("{} se conectó", username).c_str();
    {
        for (unsigned int i = 0; i < CLIENTS.size(); i++)
        {
            send(CLIENTS[i].get_socket(), encode_text(mssg, key), strlen(mssg), 0);
        }
    }
    char* buffer = (char*)calloc(1024, sizeof(1024));
    std::string msg;
    int bytes_read = 0;
    while (1)
    {
        bytes_read = recv(cliente, buffer, 1024, 0);
        if (bytes_read == -1)
        {
            break;
        }
        if (username && buffer)
        {
            msg = std::format("{}: {}", username, decode_text(buffer, key));
            for (unsigned int i = 0; i < CLIENTS.size(); i++)
            {
                send(CLIENTS[i].get_socket(), encode_text((char*)msg.c_str(), key), msg.size(), 0);
            }
        }
        msg.clear();
        memset(buffer, 0, 1024);
    }

    closesocket(cliente);
    mssg = (char*)std::format("{} se desconectó", username).c_str();
    {
        for (unsigned int i = 0; i < CLIENTS.size(); i++)
        {
            send(CLIENTS[i].get_socket(), encode_text(mssg, key), strlen(mssg), 0);
        }
    }

    for (unsigned int i = 0; i < CLIENTS.size(); i++)
    {
        if (CLIENTS[i].get_socket() == cliente)
        {
            CLIENTS.erase(CLIENTS.begin() + i);
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
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed: %d\n", WSAGetLastError());
        return 1;
    }
    std::thread file_th(file_sv);
    file_th.detach();
    listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_sock == INVALID_SOCKET) {
        printf("socket failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(SERVER_IP.c_str());
    server.sin_port = htons(5050);
    if (bind(listen_sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("bind failed: %d\n", WSAGetLastError());
        closesocket(listen_sock);
        WSACleanup();
        return 1;
    }
    printf("El servidor de chat en localhost:5050\n");
    while (true)
    {
        listen(listen_sock, SOMAXCONN);
        client_size = sizeof(struct sockaddr_in);
        client_sock = accept(listen_sock, (struct sockaddr*)&server, &client_size);
        std::thread cliente_th(chat, client_sock);
        cliente_th.detach();

    }

    WSACleanup();
    return 0;
}

