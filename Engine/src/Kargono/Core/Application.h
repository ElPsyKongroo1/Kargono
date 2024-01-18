#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Core/LayerStack.h"
#include "Kargono/Core/Window.h"
#include "Kargono/Events/Event.h"
#include "Kargono/Events/ApplicationEvent.h"
#include "Kargono/Core/ImGuiLayer.h"
#include "Kargono/Renderer/InputBuffer.h"
#include "Kargono/Physics/Physics2D.h"
#include "Kargono/Audio/AudioEngine.h"

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
		uint32_t Width, Height;
	};

	class Application
	{
	public:
		Application(const ApplicationSpecification& specification);
		virtual ~Application();

		void OnEvent(Events::Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		static Application& GetCurrentApp() { return *s_Instance; }

		void RegisterCollisionEventListener (Physics::ContactListener& contactListener)
		{
			contactListener.SetEventCallback(KG_BIND_EVENT_FN(Application::OnEvent));
		}

		void AddImGuiLayer()
		{
			KG_CORE_ASSERT(!m_ImGuiLayer, "An ImGui Layer already exists!");
			m_ImGuiLayer = new ImGuiLayer();
			PushOverlay(m_ImGuiLayer);
		}

		const ApplicationSpecification& GetSpecification() const { return m_Specification; }

		Window& GetWindow() { return *m_Window; }

		const std::filesystem::path& GetWorkingDirectory() const { return m_Specification.WorkingDirectory; }

		void Close();

		ImGuiLayer* GetImGuiLayer()
		{
			if (!m_ImGuiLayer) { KG_CORE_ERROR("Getting application ImGui Layer, but there is not ImGui Context!"); }
			return m_ImGuiLayer;
		}

		double GetAppStartTime() { return m_AppStartTime; }
		void SetAppStartTime();

		void SubmitToMainThread(const std::function<void()>& function);
		void Run();
	private:
		bool OnWindowClose(Events::WindowCloseEvent& e);
		bool OnWindowResize(Events::WindowResizeEvent& e);

		void ExecuteMainThreadQueue();
	private:
		ApplicationSpecification m_Specification;
		Scope<Window> m_Window;
		Audio::AudioEngine* m_AudioContext = nullptr;
		ImGuiLayer* m_ImGuiLayer = nullptr;
		bool m_Running = true;
		bool m_Minimized = false;
		double m_AppStartTime = 0.0f;
		LayerStack m_LayerStack;

		std::vector<std::function<void()>> m_MainThreadQueue;
		std::mutex m_MainThreadQueueMutex;

	private:
		static Application* s_Instance;
		friend int ::main(int argc, char** argv);
	};

	// To be defined in client
	Application* CreateApplication(ApplicationCommandLineArgs args);
}

