#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Core/LayerStack.h"
#include "Kargono/Core/Window.h"
#include "Kargono/Events/Event.h"
#include "Kargono/Events/ApplicationEvent.h"
#include "Kargono/ImGui/ImGuiLayer.h"
#include "Kargono/Renderer/Buffer.h"

#include <functional>
#include <mutex>
#include <filesystem>

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

	struct ApplicationSpecification
	{
		std::string Name = "Kargono Application";
		std::filesystem::path WorkingDirectory;
		ApplicationCommandLineArgs CommandLineArgs;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& specification);
		virtual ~Application();
		

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		static Application& Get() { return *s_Instance; }

		const ApplicationSpecification& GetSpecification() const { return m_Specification; }

		Window& GetWindow() { return *m_Window; }

		const std::filesystem::path& GetWorkingDirectory() const { return m_Specification.WorkingDirectory; }

		void Close();

		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

		void SubmitToMainThread(const std::function<void()>& function);
		void Run();
	private:
		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		void ExecuteMainThreadQueue();
	private:
		ApplicationSpecification m_Specification;
		Scope<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		bool m_Minimized = false;
		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.0f;

		std::vector<std::function<void()>> m_MainThreadQueue;
		std::mutex m_MainThreadQueueMutex;

	private:
		static Application* s_Instance;
		friend int ::main(int argc, char** argv);
	};

	// To be defined in client
	Application* CreateApplication(ApplicationCommandLineArgs args);
}

