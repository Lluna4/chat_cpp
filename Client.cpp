#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include <fstream>
#include <filesystem>
#include <format>
#include <WinSock2.h>
#include "Walnut/Image.h"

char* BUFFER = (char*)calloc(1024, sizeof(char));
char* USERNAME = (char*)calloc(50, sizeof(char));
std::vector<char*> MESSAGES;
SOCKET client_socket;
const std::string SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 5050;
bool FIRST = true;


static int	ft_intlen(int n)
{
	int	ret;

	ret = 1;
	while (n >= 10)
	{
		ret++;
		n = n / 10;
	}
	return (ret);
}

static char* ft_make_ret(int n, int sign)
{
	int		len;
	char* ret;

	len = ft_intlen(n) + sign;
	ret = (char *)calloc(len + 1, sizeof(char));
	if (!ret)
		return (0);
	len--;
	while (len >= 0)
	{
		ret[len] = (n % 10) + '0';
		n = n / 10;
		len--;
	}
	if (sign == 1)
		ret[0] = '-';
	return (ret);
}

char* ft_itoa(int n)
{
	char* ret;
	int		sign;

	sign = 0;
	if (n == -2147483648)
	{
		ret = (char *)malloc(12 * sizeof(char));
		if (!ret)
			return (0);
		memcpy(ret, "-2147483648", 12);
		return (ret);
	}
	if (n < 0)
	{
		n *= -1;
		sign = 1;
	}
	return (ft_make_ret(n, sign));
}


void receive(SOCKET client)
{
	while (true)
	{
		char* msg = (char*)calloc(1024, sizeof(char));
		recv(client, msg, 1024, 0);
		if (msg[0] != ' ' && msg[0] != NULL)
			MESSAGES.push_back(_strdup(msg));
		free(msg);
	}
}

class ExampleLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{
		
		ImGui::Begin("Test");

		if (MESSAGES.size() != 0)
			for (unsigned int i = 0; i < MESSAGES.size(); i++)
			{
				ImGui::Text(MESSAGES[i]);
			}
		ImGui::End();
	}
};

class ChatLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{

		ImGui::Begin("Chat");
		int textInputFlags = ImGuiInputTextFlags_EnterReturnsTrue;
		auto windowWidth = ImGui::GetWindowSize().x;

		ImGui::SetCursorPosX(windowWidth * 0.15f);
		if (ImGui::InputText(" ", BUFFER, 1024, textInputFlags)) {
			// enter was pressed 
			if (strcmp(BUFFER, "/saveuser") == 0 && std::filesystem::exists("sav") == false)
			{
				std::ofstream save("sav");
				save << USERNAME;
				save.close();
				MESSAGES.push_back(_strdup("Se ha guardado el nombre de usuario"));
			}
			else if (strcmp(BUFFER, "/saveuser") == 0 && std::filesystem::exists("sav") == true)
			{
				remove("sav");
				MESSAGES.push_back(_strdup("Se ha borrado el nombre de usuario guardado"));
			}
			else
			{
				send(client_socket, _strdup(BUFFER), strlen(BUFFER), 0);
				if (FIRST == true)
				{
					USERNAME = _strdup(BUFFER);
					FIRST = false;
					MESSAGES.pop_back();
				}
			}
			// now clear it
			memset(BUFFER, 0, sizeof(BUFFER));

			// and focus back back on that InputText widget
			ImGui::SetKeyboardFocusHere(-1);
		}
		ImGui::End();
	}
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        MESSAGES.push_back(_strdup("WSAStartup failed: "));
    }
	client_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(SERVER_IP.c_str());
    server_address.sin_port = htons(SERVER_PORT);

    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR)
    {
		MESSAGES.push_back(_strdup("Connect failed: "));
		MESSAGES.push_back(_strdup(ft_itoa(WSAGetLastError())));
        closesocket(client_socket);
        WSACleanup();
    }

    std::thread receive_thread(receive, client_socket);
    receive_thread.detach();
	Walnut::ApplicationSpecification spec;
	spec.Name = "Test chat";
	MESSAGES.push_back(_strdup("Cual es tu nombre de usuario?"));

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->PushLayer<ChatLayer>();
	return app;
}
