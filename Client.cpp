#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <string>
#include <WinSock2.h>
#include <thread>
#include <chrono>
#include <fstream>
#include <filesystem>

const std::string SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 5050;
const int MAX_USERNAME_LENGTH = 50;



void receive(SOCKET client)
{
    while (true)
    {
        char *msg = (char *)calloc(1024, sizeof(char));
        recv(client, msg, 1024, 0);
        if (msg[0] != ' ' && msg[0] != NULL)
            std::cout << msg << std::endl;
        free(msg);
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
    std::string username;
    if (std::filesystem::exists("sav") == true)
    {
        std::ifstream save("sav");
        std::getline(save, username);
    }
    else
    {
        std::cout << "Cual es tu nombre de usuario? (max " << MAX_USERNAME_LENGTH << " characters): ";
        std::getline(std::cin, username);
    }
    if (username.length() > MAX_USERNAME_LENGTH)
    {
        std::cout << "Nombre de usuario demasiado largo." << std::endl;
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }
    send(client_socket, username.c_str(), username.length() + 1, 0);

    std::thread receive_thread(receive, client_socket);
    receive_thread.detach();

    std::string message;
    while (true)
    {
        std::getline(std::cin, message);
        if (strcmp(message.c_str(), "/saveuser") == 0 && std::filesystem::exists("sav") == false)
        {
            std::ofstream save("sav");
            save << username;
            save.close();
            std::cout << "Se ha guardado el nombre de usuario" << std::endl;
            continue;
        }
        else if(strcmp(message.c_str(), "/saveuser") == 0 && std::filesystem::exists("sav") == true)
        {
            remove("sav");
            std::cout << "Se ha borrado el nombre de usuario guardado" << std::endl;
            continue;
        }

        send(client_socket, message.c_str(), message.length() + 1, 0);
    }

    closesocket(client_socket);
    WSACleanup();
    return 0;
}
