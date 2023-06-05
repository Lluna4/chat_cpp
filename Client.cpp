#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include <fstream>
#include <filesystem>
#include <format>
#include <WinSock2.h>
#include "Walnut/Image.h"
#include "encoding.hpp"
#include <ctime>
#include <commdlg.h>
#include <Windows.h>
#include <filesystem>
#include <random>

char* BUFFER = (char*)calloc(1024, sizeof(char));
std::string USERNAME;

std::vector<ImVec4> COLORS = {
	ImVec4(0.16f, 0.99f, 0.01f, 1.0f),  // Green
	ImVec4(1.0f, 0.0f, 0.0f, 1.0f),     // Red
	ImVec4(0.0f, 0.0f, 1.0f, 1.0f),     // Blue
	ImVec4(1.0f, 1.0f, 0.0f, 1.0f),     // Yellow
	ImVec4(1.0f, 0.0f, 1.0f, 1.0f),     // Magenta
	ImVec4(0.0f, 1.0f, 1.0f, 1.0f),     // Cyan
	ImVec4(0.5f, 0.2f, 0.8f, 1.0f),     // Lavender
	ImVec4(1.0f, 0.5f, 0.0f, 1.0f),     // Orange
	ImVec4(0.0f, 1.0f, 0.0f, 1.0f),     // Lime
	ImVec4(0.5f, 0.0f, 0.5f, 1.0f),     // Purple
	ImVec4(0.0f, 0.5f, 0.5f, 1.0f),     // Teal
	ImVec4(0.0f, 0.5f, 1.0f, 1.0f),     // Sky Blue
	ImVec4(0.5f, 0.0f, 0.0f, 1.0f),     // Maroon
	ImVec4(0.0f, 0.0f, 0.5f, 1.0f),     // Navy
	ImVec4(0.5f, 0.5f, 0.0f, 1.0f),     // Olive
	ImVec4(0.5f, 0.0f, 0.5f, 1.0f)      // Indigo
};
SOCKET client_socket;
const std::string SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 5050;
char* KEY = (char*)calloc(1030, sizeof(char));
bool FIRST = true;
bool FIRST_RECV = true;
int FILE_NAME = 1;

std::string open_dialog()
{
	OPENFILENAMEW ofn;               // Structure for the file dialog
	wchar_t szFile[260];             // Buffer to store the selected file path

	// Initialize the OPENFILENAME structure
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;            // Set the parent window handle (or use a valid window handle)
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
	ofn.lpstrFilter = L"Imagenes\0*.bmp;*.jpg;*.jpeg;*.png;*.gif\0";
	ofn.nFilterIndex = 1;            // Index of the default filter
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;   // Additional flags for behavior

	std::string returnValue;
	// Open the file dialog
	if (GetOpenFileNameW(&ofn) == TRUE)
	{
		MessageBox(NULL, szFile, L"Selected File", MB_OK);
		char convertedFile[MAX_PATH] = "";
		wcstombs(convertedFile, szFile, MAX_PATH);
		returnValue = convertedFile;
	}
	else
	{
		// The user canceled or an error occurred
		MessageBoxW(NULL, L"Seleccion cancelada!", L"Error", MB_OK | MB_ICONERROR);
	}
	return returnValue;
}

ImVec4 getRandomElement(const std::vector<ImVec4>& vec) {
	std::random_device rd; // Obtain a random number from hardware
	std::mt19937 eng(rd()); // Seed the generator

	// Define the range of indices
	std::uniform_int_distribution<> distr(0, vec.size() - 1);

	// Generate a random index
	int randomIndex = distr(eng);

	// Return the random element
	return vec[randomIndex];
}

char* get_time()
{
	time_t tiempo;
	time(&tiempo);
	struct tm* a = (tm*)malloc(1 * sizeof(tm));
	localtime_s(a, &tiempo);
	char* min = _strdup(ft_itoa(a->tm_min));
	if (strlen(min) == 1)
		min = ft_strjoin("0", min);
	char* time_str = (char*)std::format("{}:{}", a->tm_hour, min).c_str();
	return time_str;
}

class User
{
	public:
		User() {}

		User(char *username) 
			:username_(username)
		{}

		char* get_username()
		{
			return username_;
		}

		ImVec4 get_color()
		{
			return color;
		}

	private:
		char* username_;
		ImVec4 color = getRandomElement(COLORS); 
};

class Message
{
	public:
		Message() {}

		Message(char *msg, User usuario)
			:msg_(msg), usuario_(usuario)
		{}

		char* get_message()
		{
			return msg_;
		}

		char* get_username()
		{
			return usuario_.get_username();
		}

		ImVec4 get_color()
		{
			return usuario_.get_color();
		}

	private:
		char* msg_;
		User usuario_;

};
std::vector<Message> MESSAGES;
std::vector<User> USERS;

char* get_username(char* msg)
{
	int index = 0;
	while (msg[index] != ':')
		index++;
	char* ret = (char*)calloc(index + 1, sizeof(char));
	strncpy(ret, msg, index);
	return ret;
}


void receive(SOCKET client)
{
	while (true)
	{
		if (FIRST_RECV == true)
		{
			recv(client, KEY, 1030, 0);
			FIRST_RECV = false;
			continue;
		}
		char* msg = (char*)calloc(1024, sizeof(char));
		char* time = (char*)calloc(6, sizeof(char));
		recv(client, msg, 1024, 0);
		if (msg[0] != ' ' && msg[0] != NULL)
		{
			msg = decode_text(_strdup(msg), KEY);
			if (strstr(msg, ":") != NULL)
			{
				if (USERS.size() > 0)
				{ 
					bool found = false;
					for (unsigned int i = 0; i < USERS.size(); i++)
					{
						if (strcmp(USERS[i].get_username(), get_username(msg)) == 0)
						{
							MESSAGES.push_back(Message(_strdup(msg), USERS[i]));
							found = true;
							break;
						}
					}
					if (found == false)
					{
						User usr = User(get_username(msg));
						MESSAGES.push_back(Message(_strdup(msg), usr));
						USERS.push_back(usr);
					}
				}
				else
				{
					User usr = User(get_username(msg));
					MESSAGES.push_back(Message(_strdup(msg), usr));
					USERS.push_back(usr);
				}
				
				continue ;
			}
			MESSAGES.push_back(Message(_strdup(msg), User()));
		}
		free(msg);
		free(time);
	}
}

class ExampleLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{

		ImGui::Begin("Log");

		if (MESSAGES.size() != 0)
		{
			for (unsigned int i = 0; i < MESSAGES.size(); i++)
			{
				if (strstr(MESSAGES[i].get_message(), ":") != NULL)
				{
					if (i > 0)
					{
						ImGui::Text(" ");
						ImGui::TextColored(MESSAGES[i].get_color(), MESSAGES[i].get_username());
						ImGui::SameLine();
						ImGui::SetWindowFontScale(0.8f);
						ImGui::TextColored(ImVec4(0.38f, 0.4f, 0.42f, 0.9f), _strdup(get_time()));
						ImGui::SetWindowFontScale(1.0f);
					}
					ImGui::Text(strstr(MESSAGES[i].get_message(), " "));
				}
				else
				{
					if (strstr(MESSAGES[i].get_message(), "se conect"))
					{
						ImGui::TextColored(ImVec4(0.16f, 0.99f, 0.01f, 1.0f), MESSAGES[i].get_message());
					}
					else
					{
						ImGui::Text(MESSAGES[i].get_message());
					}
					//m_image = std::make_shared<Walnut::Image>("test.png");
					//ImGui::Image(m_image->GetDescriptorSet(), {(float)m_image->GetWidth(), (float)m_image->GetHeight()});
				}

			}
		}
		ImGui::End();
	}
	/*private:
		std::shared_ptr<Walnut::Image> m_image;*/
};

class ChatLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{

		ImGui::Begin("Chat");
		if (FIRST == true)
		{
			ImGui::OpenPopup("username");
		}
		if (ImGui::BeginPopupModal("username")) 
		{
			ImGui::Text("Cual es tu nombre de usuario?");
			int textInputFlags = ImGuiInputTextFlags_EnterReturnsTrue;
			if (ImGui::InputText(" ", BUFFER, 1024, textInputFlags))
			{
				USERNAME = _strdup(BUFFER);
				memset(BUFFER, 0, sizeof(BUFFER));
				FIRST = false;
				send(client_socket, encode_text((char *)USERNAME.c_str(), KEY), USERNAME.size(), 0);
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		int textInputFlags = ImGuiInputTextFlags_EnterReturnsTrue;
		auto windowWidth = ImGui::GetWindowSize().x;

		ImGui::SetCursorPosX(windowWidth * 0.15f);
		if (ImGui::InputText(" ", BUFFER, 1024, textInputFlags)) 
		{
			// enter was pressed 
			if (strcmp(BUFFER, "/fps") == 0)
			{
				MESSAGES.push_back(Message(_strdup(ft_itoa((int)ImGui::GetIO().Framerate)), User()));
			}
			else
			{
				send(client_socket, encode_text(_strdup(BUFFER), KEY), strlen(BUFFER), 0);
			}
			// now clear it
			memset(BUFFER, 0, sizeof(BUFFER));

			ImGui::SetKeyboardFocusHere(-1);
		}
		ImGui::SameLine();
		if (ImGui::Button("test"))
		{
			std::string file_path = open_dialog();
			FILE* fd = fopen(file_path.c_str(), "rb");
			size_t rret, wret;
			int bytes_read;
			char buffer[1024];
			send(client_socket, encode_text((char *)"/image", KEY), 6, 0);
			while (!feof(fd)) {
				if ((bytes_read = fread(&buffer, 1, 1024, fd)) > 0)
					send(client_socket, buffer, bytes_read, 0);
				else
					break;
			}
		}
		ImGui::End();
	}
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup failed: ";
    }
	client_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(SERVER_IP.c_str());
    server_address.sin_port = htons(SERVER_PORT);

    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR)
    {
		std::cerr << "Connect failed: " << WSAGetLastError();
        closesocket(client_socket);
        WSACleanup();
    }

    std::thread receive_thread(receive, client_socket);
    receive_thread.detach();

	if (std::filesystem::exists("sav") == true)
	{
		std::ifstream save("sav");
		std::getline(save, USERNAME);
		send(client_socket, encode_text((char*)USERNAME.c_str(), KEY), USERNAME.size(), 0);
		FIRST = false;
	}
	Walnut::ApplicationSpecification spec;
	spec.Name = "Test chat";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->PushLayer<ChatLayer>();
	app->SetMenubarCallback([app]()
		{
			if (ImGui::BeginMenu("Guardar"))
			{
				if (ImGui::MenuItem("Guardar usuario"))
				{
					std::ofstream save("sav");
					save << USERNAME;
					save.close();
					MESSAGES.push_back(Message(_strdup("Se ha guardado el nombre de usuario"), User()));
				}
				if (ImGui::MenuItem("Eliminar usuario"))
				{
					if (std::filesystem::exists("sav") == true)
					{
						remove("sav");
						MESSAGES.push_back(Message(_strdup("Se ha borrado el nombre de usuario guardado"), User()));
					}
				}
				ImGui::EndMenu();
			}
		}
	);
	return app;
}
