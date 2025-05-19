#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Core/Application.h"
#include "Kargono/Core/Window.h"
#include "Modules/Events/Event.h"
#include "Modules/Events/ApplicationEvent.h"
#include "Modules/Rendering/InputBuffer.h"
#include "Modules/Physics2D/Physics2D.h"
#include "Modules/Audio/Audio.h"
#include "Modules/Events/NetworkingEvent.h"
#include "Modules/Events/EventQueue.h"
#include "Kargono/Core/FunctionQueue.h"
#include "Kargono/Utility/Timers.h"
#include "Kargono/Core/FixedString.h"
#include "Kargono/Utility/Timers.h"

#include <functional>
#include <mutex>
#include <atomic>
#include <filesystem>
#include <chrono>

namespace Kargono
{
	struct CommandLineArguments
	{
		//==============================
		// Public Fields
		//==============================
		int m_ArgCount = 0;
		char** m_ArgArray{ nullptr };

		//==============================
		// Operator Overloads
		//==============================
		const char* operator[](int index) const
		{
			KG_ASSERT(index < m_ArgCount);
			return m_ArgArray[index];
		}
	};

	struct EngineConfig
	{
		FixedString32 m_ExecutableName{};
		std::filesystem::path m_WorkingDirectory{};
		CommandLineArguments m_CmlArgs{};
		Math::uvec2 m_DefaultWindowDimensions{0, 0};
	};

	class EngineService;
	class Engine;

	class EngineThread
	{
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		[[nodiscard]] bool Init(Application* currentApp, Window* currentWindow);
		[[nodiscard]] bool Terminate(bool withinThread);
	private:
		// Terminate helper
		void ClearThreadResources();
	public:
		//==============================
		// Run Thread
		//==============================
		void RunThread();
		void EndThread();

		//==============================
		// On Event
		//==============================
		void OnEvent(Events::Event* e);
	private:
		// Custom event handlers
		bool OnWindowClose(Events::WindowCloseEvent& e);
		bool OnWindowResize(Events::WindowResizeEvent& e);
		bool OnUpdateEntityLocation(Events::UpdateEntityLocation& e);
		bool OnUpdateEntityPhysics(Events::UpdateEntityPhysics& e);
		void OnSkipUpdate(Events::SkipUpdateEvent event);
		void OnAddExtraUpdate(Events::AddExtraUpdateEvent event);
		bool OnCleanUpTimers(Events::CleanUpTimersEvent& e);
		bool OnAddTickGeneratorUsage(Events::AddTickGeneratorUsage& e);
		bool OnRemoveTickGeneratorUsage(Events::RemoveTickGeneratorUsage& e);
		bool OnAppTickEvent(Events::AppTickEvent& e);
		void OnApplicationCloseEvent(Events::ApplicationCloseEvent& e);
	public:
		//==============================
		// Work Queues
		//==============================
		void SubmitFunction(const std::function<void()> workFunction);
		void SubmitEvent(Ref<Events::Event> event);

		//==============================
		// Getters/Setters
		//==============================
		UpdateCount GetUpdateCount() { return m_RunTimer.GetUpdateCount(); }
		float GetInApplicationTime() const;
		double GetAppStartTime() const { return m_EngineStartTime; };
		void UpdateAppStartTime();
	private:
		//==============================
		// Internal Fields
		//==============================
		// Main timer
		Utility::LoopTimer m_RunTimer{};
		// Thread state/config
		bool m_Active{ false };
		double m_EngineStartTime{ 0.0f };
		// Work queues
		FunctionQueue m_WorkQueue{};
		Events::EventQueue m_EventQueue{};

	private:
		//==============================
		// Injected Dependencies
		//==============================
		Application* i_EngineApp{ nullptr };
		Window* i_EngineWindow{ nullptr };
	};

	class Engine
	{
	public:
		//==============================
		// Constructor/Destructor
		//==============================
		Engine() = default;
		~Engine() = default;
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		[[nodiscard]] bool Init(const EngineConfig& config, Application* app);
		[[nodiscard]] bool Terminate();

		//==============================
		// Register Event Callbacks
		//==============================
		void RegisterWindowOnEventCallback();
		void RegisterAppTickOnEventCallback();
		void RegisterCollisionEventListener(Physics::ContactListener& contactListener);
	public:
		//==============================
		// Getters/Setters
		//==============================
		const EngineConfig& GetConfig() const { return m_Config; }
		Window& GetWindow() { return *m_Window; }
		EngineThread& GetThread() { return m_Thread; }
		Application& GetApp() { return *m_CurrentApp; }

		[[nodiscard]] bool IsApplicationActive() // TODO: PLEASE REMOVE THIS
		{
			if (!m_Active)
			{
				return false;
			}

			if (!m_CurrentApp)
			{
				return false;
			}

			return true;
		}
	private:
		//==============================
		// Internal Fields
		//==============================
		// Initialization Data
		bool m_Active{ false };
		EngineConfig m_Config;
		// Thread context
		EngineThread m_Thread;
		// Engine Data
		Scope<Window> m_Window{ nullptr };
		Application* m_CurrentApp{ nullptr };
	private:
		friend EngineService;
	};

	class EngineService
	{
	public:
		//==============================
		// Getters/Setters
		//==============================
		static void SetActiveEngine(Engine* engine)
		{
			KG_ASSERT(engine);

			s_ActiveEngine = engine;
		}
		static Engine& GetActiveEngine() { return *s_ActiveEngine; }
		static bool IsEngineActive() { return (bool)s_ActiveEngine; }
	private:
		//==============================
		// Internal Fields
		//==============================
		static inline Engine* s_ActiveEngine{ nullptr };
	private:
		friend Engine;
	};

	// To be defined in client
	void ExecutableEntryPoint(Engine& engine, CommandLineArguments args);
}

