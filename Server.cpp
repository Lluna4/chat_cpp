#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
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

    Message(char* username, char *message, bool sent)
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

    char *get_message()
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

    int get_size()
    {
        return strlen(username_) + strlen(message_);
    }

private:
    char* username_;
    char *message_;
    bool sent_;
};


std::vector<Cliente> CLIENTS;
std::vector<Message> MESSAGES;

void push_to_db()
{
    std::ofstream db("./resources/db");
    for (unsigned int i = 0; i < MESSAGES.size(); i++)
    {
        db << std::format("{} {}\n", MESSAGES[i].get_username(), MESSAGES[i].get_message());       
    }
    std::cout << "Guardado!" << std::endl;
    db.close();
}

void db()
{
    int prev_size = 0;
    if (std::filesystem::exists("./resources") == false)
    {
        std::filesystem::create_directories("./resources");
        std::cout << "Creada carpeta resources" << std::endl;
    }
    while (1)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10));
        if ((int)MESSAGES.size() > prev_size)
        {
            prev_size = (int)MESSAGES.size();
            push_to_db();
        }
    }
}

void file_sv(SOCKET client)
{
    int bytesReceived = 1;
    char buffer[4096];
    int receive = 0;
    char* file_name = (char*)calloc(10, sizeof(char));
    char *file_num_buff = (char *)calloc(2, sizeof(char));
    recv(client, file_num_buff, 2, 0);
    std::cout << atoi(file_num_buff) << std::endl;
    recv(client, file_name, atoi(file_num_buff), 0);
    std::ofstream output_file(file_name, std::ios::binary);
    while (bytesReceived > 0)
    {
        bytesReceived = recv(client, buffer, 4096, 0);
        if (bytesReceived > 0)
            output_file.write(buffer, bytesReceived);
    }
    output_file.flush();
    output_file.close();
    closesocket(client);
}


void file_sv_listen()
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
        std::thread file_sv_th(file_sv, client_sock);
        file_sv_th.detach();
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
    char* mssg = (char*)std::format("{} se conecto", username).c_str();
    for (unsigned int i = 0; i < CLIENTS.size(); i++)
    {
        send(CLIENTS[i].get_socket(), encode_text(mssg, key), strlen(mssg), 0);
    }
    char* buffer = (char*)calloc(1024, sizeof(1024));
    std::string msg;
    int bytes_read = 0;
    std::cout << MESSAGES.size() << std::endl;
    for (unsigned int i = 0; i < MESSAGES.size(); i++)
    {
        msg = std::format("{}: {}", MESSAGES[i].get_username(), MESSAGES[i].get_message());
        std::cout << msg << std::endl;
        send(cliente, encode_text((char*)msg.c_str(), key), msg.size(), 0);
    }
    while (1)
    {
        bytes_read = recv(cliente, buffer, 1024, 0);
        if (bytes_read == -1)
        {
            break;
        }
        if (username && buffer)
        {
            MESSAGES.push_back(Message(username, decode_text(buffer, key), true));
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
    mssg = (char*)std::format("{} se desconecto", username).c_str();
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


void chat_listen()
{
    int recv_size, client_size;
    SOCKET listen_sock, client_sock;
    struct sockaddr_in server;
    listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_sock == INVALID_SOCKET) {
        printf("socket failed: %d\n", WSAGetLastError());
        WSACleanup();
        return ;
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(SERVER_IP.c_str());
    server.sin_port = htons(5050);
    if (bind(listen_sock, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("bind failed: %d\n", WSAGetLastError());
        closesocket(listen_sock);
        WSACleanup();
        return ;
    }
    printf("El servidor de chat en localhost:5050\n");
    if (std::filesystem::exists("./resources/db"))
    {
        std::ifstream rec("./resources/db");
        std::string a, b;
        while (rec >> a >> b)
        {
            MESSAGES.push_back(Message(_strdup(a.c_str()), _strdup(b.c_str()), true));
        }
        std::cout << "Cargada base de datos " << MESSAGES.size() << std::endl;
    }
    std::thread db_th(db);
    db_th.detach();
    while (true)
    {
        listen(listen_sock, SOMAXCONN);
        client_size = sizeof(struct sockaddr_in);
        client_sock = accept(listen_sock, (struct sockaddr*)&server, &client_size);
        std::thread cliente_th(chat, client_sock);
        cliente_th.detach();
    }

    WSACleanup();
}

int main()
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed: %d\n", WSAGetLastError());
        return 1;
    }
    std::thread file_th(file_sv_listen);
    file_th.detach();
    std::thread listen_th(chat_listen);
    char msg[1024];
    while (true)
    {
        std::cin.getline(msg, 1024);
        if (strcmp(msg, "stop") == 0)
        {
            exit(0);
        }
    }
    return 0;
}

