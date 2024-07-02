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

namespace Kargono
{
	//==============================
	// Command Line Arguments Struct
	//==============================
	struct CommandLineArguments
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
	struct EngineSpec
	{
		std::string Name = "Kargono Application";
		std::filesystem::path WorkingDirectory{};
		CommandLineArguments CommandLineArgs{};
		uint32_t DefaultWindowHeight{ 0 };
		uint32_t DefaultWindowWidth{ 0 };
	};

	class EngineService;
	//==============================
	// Engine Class
	//==============================
	class Engine
	{
	private: // (Singleton)
		//==============================
		// Constructor/Destructor
		//==============================
		Engine() = default;
		~Engine() = default;
	public:
		//==============================
		// Getters/Setters
		//==============================
		const EngineSpec& GetSpecification() const { return m_Specification; }
		Window& GetWindow() { return *m_Window; }
		const std::filesystem::path& GetWorkingDirectory() const { return m_Specification.WorkingDirectory; }
		double GetAppStartTime() const { return m_AppStartTime; }
		uint64_t GetUpdateCount() const { return m_UpdateCount; }
		void UpdateAppStartTime();

	private:
		//==============================
		// Internal Fields
		//==============================
		// Initialization Data
		EngineSpec m_Specification;
		// Engine Data
		Scope<Window> m_Window{ nullptr };
		Application* m_CurrentApp{ nullptr };
		// Run State Data
		bool m_Running { true };
		bool m_Minimized { false };
		double m_AppStartTime = 0.0f;
		std::chrono::nanoseconds m_Accumulator{0};
		std::atomic<uint64_t> m_UpdateCount {0};
		// Event/Function Queues
		std::vector<std::function<void()>> m_MainThreadQueue;
		std::mutex m_MainThreadQueueMutex;
		std::vector<Ref<Events::Event>> m_EventQueue {};
		std::mutex m_EventQueueMutex;
		
	private:
		friend EngineService;
	};

	//==============================
	// Engine Service Class
	//==============================
	class EngineService
	{
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		static void Init(const EngineSpec& specification, Application* app);
		static void Terminate();

		static void Run();
		static void EndRun();
		//==============================
		// OnEvent Functions
		//==============================
		static void OnEvent(Events::Event& e);
		static bool OnWindowClose(Events::WindowCloseEvent& e);
		static bool OnWindowResize(Events::WindowResizeEvent& e);
		static bool OnUpdateEntityLocation(Events::UpdateEntityLocation& e);
		static bool OnUpdateEntityPhysics(Events::UpdateEntityPhysics& e);
		static void OnSkipUpdate(Events::SkipUpdateEvent event);
		static void OnAddExtraUpdate(Events::AddExtraUpdateEvent event);
		static bool OnCleanUpTimers(Events::CleanUpTimersEvent& e);
		static bool OnAddTickGeneratorUsage(Events::AddTickGeneratorUsage& e);
		static bool OnRemoveTickGeneratorUsage(Events::RemoveTickGeneratorUsage& e);
		static bool OnAppTickEvent(Events::AppTickEvent& e);

		//==============================
		// Register Event Callbacks
		//==============================
		static void RegisterWindowOnEventCallback();
		static void RegisterAppTickOnEventCallback();
		static void RegisterCollisionEventListener(Physics::ContactListener& contactListener);
		//==============================
		// Submit to Event/Function Queues
		//==============================
		static void SubmitToMainThread(const std::function<void()>& function);
		static void SubmitToEventQueue(Ref<Events::Event> e);
		static void SubmitApplicationCloseEvent();
		//==============================
		// Getters/Setters
		//==============================
		static Engine& GetActiveEngine() { return *s_ActiveEngine; }
		static Window& GetActiveWindow() { return s_ActiveEngine->GetWindow(); }
	private:
		static void ExecuteMainThreadQueue();
		static void ProcessEventQueue();
	private:
		//==============================
		// Internal Fields
		//==============================
		static Engine* s_ActiveEngine;
	private:
		friend Engine;
	};

	// To be defined in client
	void InitEngineAndCreateApp(CommandLineArguments args);
}

