#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Core/Application.h"
#include "Kargono/Core/Window.h"
#include "Kargono/Events/Event.h"
#include "Kargono/Events/ApplicationEvent.h"
#include "Kargono/Rendering/InputBuffer.h"
#include "Kargono/Physics/Physics2D.h"
#include "Kargono/Audio/Audio.h"
#include "Kargono/Events/NetworkingEvent.h"

#include <functional>
#include <mutex>
#include <atomic>
#include <filesystem>
#include <chrono>

int main(int argc, char** argv);

namespace Kargono
{
	//==============================
	// Command Line Arguments Struct
	//==============================
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

	//==============================
	// Application Specification Struct
	//==============================
	struct AppSpec
	{
		std::string Name = "Kargono Application";
		std::filesystem::path WorkingDirectory;
		CommandLineArgs CommandLineArgs;
		uint32_t Width, Height;
	};
	//==============================
	// Engine Core Struct	
	//==============================
	class EngineCore
	{
	public:
		//==============================
		// Constructor/Destructor
		//==============================
		EngineCore(const AppSpec& specification, Application* app);
		~EngineCore();
	public:
		//==============================
		// LifeCycle Functions
		//==============================
		void Close();
		void Run();
	public:
		//==============================
		// Event Functions
		//==============================
		void OnEvent(Events::Event& e);
		bool OnWindowClose(Events::WindowCloseEvent& e);
		bool OnWindowResize(Events::WindowResizeEvent& e);
		bool OnUpdateEntityLocation(Events::UpdateEntityLocation& e);
		bool OnUpdateEntityPhysics(Events::UpdateEntityPhysics& e);
		void OnSkipUpdate(Events::SkipUpdateEvent event);
		void OnAddExtraUpdate(Events::AddExtraUpdateEvent event);
		void RegisterCollisionEventListener (Physics::ContactListener& contactListener)
		{
			contactListener.SetEventCallback(KG_BIND_CLASS_FN(EngineCore::OnEvent));
		}
	private:
		bool OnCleanUpTimers(Events::CleanUpTimersEvent& e);
		bool OnAddTickGeneratorUsage(Events::AddTickGeneratorUsage& e);
		bool OnRemoveTickGeneratorUsage(Events::RemoveTickGeneratorUsage& e);
		bool OnAppTickEvent(Events::AppTickEvent& e);
	public:
		//==============================
		// Submission API
		//==============================
		void SubmitToMainThread(const std::function<void()>& function);
		void SubmitToEventQueue(Ref<Events::Event> e);

	public:
		//==============================
		// Getters/Setters
		//==============================
		static EngineCore& GetCurrentEngineCore() { return *s_CurrentEngineCore; }
		static Window& GetActiveWindow() { return *(s_CurrentEngineCore->m_Window); }
		const AppSpec& GetSpecification() const { return m_Specification; }
		Window& GetWindow() { return *m_Window; }
		const std::filesystem::path& GetWorkingDirectory() const { return m_Specification.WorkingDirectory; }
		double GetAppStartTime() const { return m_AppStartTime; }
		uint64_t GetUpdateCount() const { return m_UpdateCount; }
		void SetAppStartTime();

		//==============================
		// Internal Functionality
		//==============================
		void ExecuteMainThreadQueue();
		void ProcessEventQueue();
	private:
		// Initialization Data
		AppSpec m_Specification;
		// Engine State Data
		Scope<Window> m_Window;
		Application* m_CurrentApp{ nullptr };
		Audio::AudioService* m_AudioContext = nullptr;
		bool m_Running = true;
		bool m_Minimized = false;
		double m_AppStartTime = 0.0f;
		std::chrono::nanoseconds m_Accumulator{0};
		std::atomic<uint64_t> m_UpdateCount = 0;
		// Event/Function Queues
		std::vector<std::function<void()>> m_MainThreadQueue;
		std::mutex m_MainThreadQueueMutex;
		std::vector<Ref<Events::Event>> m_EventQueue {};
		std::mutex m_EventQueueMutex;
		// Static Resources
		static EngineCore* s_CurrentEngineCore;
	private:
		friend int ::main(int argc, char** argv);
	};

	// To be defined in client
	EngineCore* InitEngineAndCreateApp(CommandLineArgs args);
}

