#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include <fstream>
#include <filesystem>
#include <format>
#include "Walnut/Image.h"

char* BUFFER = (char*)calloc(1024, sizeof(char));
char* USERNAME = _strdup("Luna");
std::vector<char*> MESSAGES;

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
		else
			ImGui::Text("PRUEBA");

		
		auto windowHeight = ImGui::GetWindowSize().y;
		auto windowWidth = ImGui::GetWindowSize().x;

		ImGui::SetCursorPosY(windowHeight * 0.8f);
		ImGui::SetCursorPosX(windowWidth * 0.1f);
		int textInputFlags = ImGuiInputTextFlags_EnterReturnsTrue;
		if (ImGui::InputText("test", BUFFER, 1024, textInputFlags)) {
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
				MESSAGES.push_back(_strdup((char*)std::format("{}: {}\n", USERNAME, BUFFER).c_str()));
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
	Walnut::ApplicationSpecification spec;
	spec.Name = "Test chat";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	return app;
}
