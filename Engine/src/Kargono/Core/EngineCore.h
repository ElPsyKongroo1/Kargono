#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Core/Application.h"
#include "Kargono/Core/Window.h"
#include "Kargono/Events/Event.h"
#include "Kargono/Events/ApplicationEvent.h"
#include "Kargono/Renderer/InputBuffer.h"
#include "Kargono/Physics/Physics2D.h"
#include "Kargono/Audio/AudioEngine.h"

#include <functional>
#include <mutex>
#include <atomic>
#include <filesystem>
#include <chrono>

#include "Kargono/Events/NetworkingEvent.h"


int main(int argc, char** argv);

namespace Kargono
{

	struct CommandLineArgs
	{
		int Count = 0;
		char** Args = nullptr;

		const char* operator[](int index) const
		{
			KG_ASSERT(index < Count, "Invalid attempt to access command line arguments");
			return Args[index];
		}
	};

	struct AppSpec
	{
		std::string Name = "Kargono Application";
		std::filesystem::path WorkingDirectory;
		CommandLineArgs CommandLineArgs;
		uint32_t Width, Height;
	};

	class EngineCore
	{
	public:
		EngineCore(const AppSpec& specification, Application* app);
		~EngineCore();
		void OnEvent(Events::Event& e);

		static EngineCore& GetCurrentApp() { return *s_Instance; }

		void RegisterCollisionEventListener (Physics::ContactListener& contactListener)
		{
			contactListener.SetEventCallback(KG_BIND_EVENT_FN(EngineCore::OnEvent));
		}


		const AppSpec& GetSpecification() const { return m_Specification; }

		Window& GetWindow() { return *m_Window; }

		const std::filesystem::path& GetWorkingDirectory() const { return m_Specification.WorkingDirectory; }

		void Close();


		double GetAppStartTime() const { return m_AppStartTime; }
		uint64_t GetUpdateCount() const { return m_UpdateCount; }
		void SetAppStartTime();

		void SubmitToMainThread(const std::function<void()>& function);

		void SubmitToEventQueue(Ref<Events::Event> e);

		void OnSkipUpdate(Events::SkipUpdateEvent event);
		void OnAddExtraUpdate(Events::AddExtraUpdateEvent event);

		void Run();
	private:
		bool OnWindowClose(Events::WindowCloseEvent& e);
		bool OnWindowResize(Events::WindowResizeEvent& e);
		bool OnCleanUpTimers(Events::CleanUpTimersEvent& e);
		bool OnAddTickGeneratorUsage(Events::AddTickGeneratorUsage& e);
		bool OnRemoveTickGeneratorUsage(Events::RemoveTickGeneratorUsage& e);
		bool OnAppTickEvent(Events::AppTickEvent& e);
		bool OnUpdateEntityLocation(Events::UpdateEntityLocation& e);
		bool OnUpdateEntityPhysics(Events::UpdateEntityPhysics& e);
		void ExecuteMainThreadQueue();
		void ProcessEventQueue();
	private:
		AppSpec m_Specification;
		Application* m_CurrentApp{ nullptr };
		Scope<Window> m_Window;
		Audio::AudioEngine* m_AudioContext = nullptr;
		bool m_Running = true;
		bool m_Minimized = false;
		double m_AppStartTime = 0.0f;
		std::chrono::nanoseconds m_Accumulator{0};
		std::atomic<uint64_t> m_UpdateCount = 0;

		std::vector<std::function<void()>> m_MainThreadQueue;
		std::mutex m_MainThreadQueueMutex;

		std::vector<Ref<Events::Event>> m_EventQueue {};
		std::mutex m_EventQueueMutex;

	private:
		static EngineCore* s_Instance;
		friend int ::main(int argc, char** argv);
	};

	// To be defined in client
	EngineCore* InitEngineAndCreateApp(CommandLineArgs args);
}

