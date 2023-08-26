#pragma once


#include "Kargono/Core/Base.h"

#include "Kargono/Core/LayerStack.h"
#include "Kargono/Core/Window.h"
#include "Kargono/Events/Event.h"
#include "Kargono/Events/ApplicationEvent.h"
#include "Kargono/ImGui/ImGuiLayer.h"
#include "Kargono/Renderer/Buffer.h"

#include "Kargono/Core/Timestep.h"

int main(int argc, char** argv);

namespace Kargono
{

	struct ApplicationCommandLineArgs
	{
		int Count = 0;
		char** Args = nullptr;

		const char* operator[](int index) const
		{
			KG_CORE_ASSERT(index < Count, "Invalid attempt to access command line arguments");
			return Args[index];
		}
	};

	class Application
	{
	public:
		Application(const std::string& name = "Kargono App", ApplicationCommandLineArgs args = ApplicationCommandLineArgs());
		virtual ~Application();
		

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		static Application& Get() { return *s_Instance; }

		ApplicationCommandLineArgs GetCommandLineArgs() const { return m_CommandLineArgs; }

		Window& GetWindow() { return *m_Window; }

		void Close();

		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }
	private:
		void Run();
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);
	private:
		ApplicationCommandLineArgs m_CommandLineArgs;
		Scope<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		bool m_Minimized = false;
		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.0f;

	private:
		static Application* s_Instance;
		friend int ::main(int argc, char** argv);
	};

	// To be defined in client
	Application* CreateApplication(ApplicationCommandLineArgs args);
}

