#include "kgpch.h"

#include "Kargono/Core/Engine.h"

#include "Kargono/Core/AppTick.h"
#include "Kargono/Utility/Time.h"
#include "Physics2DPlugin/Physics2D.h"
#include "RenderingPlugin/RenderingService.h"
#include "Kargono/Core/Profiler.h"
#include "Kargono/Utility/Timers.h"
#include "EventsPlugin/NetworkingEvent.h"
#include "ECSPlugin/Entity.h"
#include "Kargono/Scenes/Scene.h"
#include "Physics2DPlugin/Physics2D.h"
#include "NetworkPlugin/Client.h"
#include "ScriptingPlugin/ScriptService.h"


namespace Kargono
{
//#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	std::chrono::nanoseconds k_ConstantFrameTime { 1'000 * 1'000 * 1'000 / 60 };
	Timestep k_ConstantFrameTimeStep { 1.0f / 60.0f };

	float Engine::GetInApplicationTime() const
	{
		return (float)m_UpdateCount * k_ConstantFrameTimeStep;
	}

	void Engine::UpdateAppStartTime()
	{
		m_AppStartTime = Utility::Time::GetTime();
	}
	
	void EngineService::Init(const EngineSpec& specification, Application* app)
	{
		// Ensure Engine is a Singleton
		KG_ASSERT(!s_ActiveEngine, "Application already exists!")

		// Initialize engine
		s_ActiveEngine = new Engine();
		KG_VERIFY(s_ActiveEngine, "Engine Initialized");
		s_ActiveEngine->m_Specification = specification;
		s_ActiveEngine->m_CurrentApp = app;

		// Initialize main window
		s_ActiveEngine->m_Window = Window::Create(WindowProps(s_ActiveEngine->m_Specification.Name, 
			s_ActiveEngine->m_Specification.DefaultWindowWidth, 
			s_ActiveEngine->m_Specification.DefaultWindowHeight));
		KG_VERIFY(s_ActiveEngine->m_Window, "Create Window");
		RegisterWindowOnEventCallback();
		RegisterAppTickOnEventCallback();

		// Set up event queue
		s_ActiveEngine->m_EventQueue.Init(OnEvent);

		// Initialize current app (editor, runtime, server, etc...)
		app->Init();
	}

	bool EngineService::Terminate()
	{
		if (!s_ActiveEngine)
		{
			KG_WARN("Attempt to terminate engine service when s_ActiveEngine is already closed");
			return true;
		}

		if (s_ActiveEngine->m_CurrentApp)
		{
			s_ActiveEngine->m_CurrentApp->Terminate();
			delete s_ActiveEngine->m_CurrentApp;
			s_ActiveEngine->m_CurrentApp = nullptr;
		}
		KG_VERIFY(!s_ActiveEngine->m_CurrentApp, "Application Terminated");

		delete s_ActiveEngine;
		s_ActiveEngine = nullptr;
		KG_VERIFY(!s_ActiveEngine, "Active Engine Terminated");
		return true;
		
	}

	void EngineService::Run()
	{
		KG_INFO("Starting Run Function");

		if (!s_ActiveEngine)
		{
			return;
		}

		using namespace std::chrono_literals;

		std::chrono::time_point<std::chrono::high_resolution_clock> currentTime = std::chrono::high_resolution_clock::now();
		std::chrono::time_point<std::chrono::high_resolution_clock> lastCycleTime = currentTime;
		std::chrono::nanoseconds timestep{ 0 };

		while (s_ActiveEngine->m_Running)
		{
			currentTime = std::chrono::high_resolution_clock::now();
			timestep = currentTime - lastCycleTime;
			lastCycleTime = currentTime;
			s_ActiveEngine->m_Accumulator += timestep;
			if (s_ActiveEngine->m_Accumulator < k_ConstantFrameTime)
			{
				continue;
			}
			s_ActiveEngine->m_Accumulator -= k_ConstantFrameTime;

			{
				KG_PROFILE_FRAME_DESC("Main Thread");

				s_ActiveEngine->m_UpdateCount++;
				AppTickService::OnUpdate(k_ConstantFrameTimeStep);
				Utility::PassiveTimer::OnUpdate(k_ConstantFrameTimeStep);

				ProcessFunctionQueue();
				if (!s_ActiveEngine->m_Minimized)
				{
					s_ActiveEngine->m_CurrentApp->OnUpdate(k_ConstantFrameTimeStep);
				}
				ProcessEventQueue();
				s_ActiveEngine->m_Window->OnUpdate();
			}
		}
		KG_INFO("Ending Run Function");
	}

	void EngineService::EndRun()
	{
		s_ActiveEngine->m_Running = false;
	}

	void EngineService::OnEvent(Events::Event* e)
	{
		bool handled = false;
		// Process Input Events
		if (e->IsInCategory(Events::Input))
		{
			if (s_ActiveEngine->m_CurrentApp)
			{
				s_ActiveEngine->m_CurrentApp->OnInputEvent(e);
			}
			return;
		}

		// Process Physics Events
		if (e->IsInCategory(Events::Physics))
		{
			if (s_ActiveEngine->m_CurrentApp)
			{
				s_ActiveEngine->m_CurrentApp->OnPhysicsEvent(e);
			}
			return;
		}

		// Process Network Events
		if (e->IsInCategory(Events::Network))
		{
			switch (e->GetEventType())
			{
			case Events::EventType::UpdateEntityLocation:
				handled = OnUpdateEntityLocation(*(Events::UpdateEntityLocation*)e);
				break;
			case Events::EventType::UpdateEntityPhysics:
				handled = OnUpdateEntityPhysics(*(Events::UpdateEntityPhysics*)e);
				break;
			}

			if (handled)
			{
				return;
			}
			if (s_ActiveEngine->m_CurrentApp)
			{
				s_ActiveEngine->m_CurrentApp->OnNetworkEvent(e);
			}
			return;
		}

		if (e->IsInCategory(Events::Scene))
		{
			if (s_ActiveEngine->m_CurrentApp)
			{
				s_ActiveEngine->m_CurrentApp->OnSceneEvent(e);
			}
			return;
		}

		if (e->IsInCategory(Events::Asset))
		{
			if (s_ActiveEngine->m_CurrentApp)
			{
				s_ActiveEngine->m_CurrentApp->OnAssetEvent(e);
			}
			return;
		}

		// Process Application Events
		if (e->IsInCategory(Events::Application))
		{
			switch (e->GetEventType())
			{
			case Events::EventType::LogEvent:
				handled = s_ActiveEngine->m_CurrentApp->OnLogEvent(e);
				break;
			case Events::EventType::WindowClose:
				handled = OnWindowClose(*(Events::WindowCloseEvent*)e);
				break;
			case Events::EventType::WindowResize:
				handled = OnWindowResize(*(Events::WindowResizeEvent*)e);
				break;
			case Events::EventType::CleanUpTimers:
				handled = OnCleanUpTimers(*(Events::CleanUpTimersEvent*)e);
				break;
			case Events::EventType::AddTickGeneratorUsage:
				handled = OnAddTickGeneratorUsage(*(Events::AddTickGeneratorUsage*)e);
				break;
			case Events::EventType::RemoveTickGeneratorUsage:
				handled = OnRemoveTickGeneratorUsage(*(Events::RemoveTickGeneratorUsage*)e);
				break;
			case Events::EventType::AppTick:
				handled = OnAppTickEvent(*(Events::AppTickEvent*)e);
				break;
			}

			if (handled)
			{
				return;
			}

			if (s_ActiveEngine->m_CurrentApp)
			{
				s_ActiveEngine->m_CurrentApp->OnApplicationEvent(e);
			}

			return;
		}

		if (e->IsInCategory(Events::Editor))
		{
			if (s_ActiveEngine->m_CurrentApp)
			{
				s_ActiveEngine->m_CurrentApp->OnEditorEvent(e);
			}
			return;
		}

	}

	bool EngineService::OnWindowClose(Events::WindowCloseEvent& e)
	{
		UNREFERENCED_PARAMETER(e);

		s_ActiveEngine->m_Running = false;
		return true;
	}

	bool EngineService::OnWindowResize(Events::WindowResizeEvent& e)
	{
		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			s_ActiveEngine->m_Minimized = true;
			return false;
		}

		s_ActiveEngine->m_Minimized = false;
		Rendering::RenderingService::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;
	}

	bool EngineService::OnUpdateEntityLocation(Events::UpdateEntityLocation& e)
	{
		Ref<Scenes::Scene> scene = Scenes::SceneService::GetActiveScene();
		if (!scene) { return false; }
		ECS::Entity entity = scene->GetEntityByUUID(e.GetEntityID());
		if (!entity) { return false; }
		Math::vec3 translation = e.GetTranslation();
		entity.GetComponent<ECS::TransformComponent>().Translation = translation;

		if (entity.HasComponent<ECS::Rigidbody2DComponent>())
		{
			auto& rb2d = entity.GetComponent<ECS::Rigidbody2DComponent>();
			b2Body* body = (b2Body*)rb2d.RuntimeBody;
			body->SetTransform({ translation.x, translation.y }, body->GetAngle());
		}
		return false;
	}

	bool EngineService::OnUpdateEntityPhysics(Events::UpdateEntityPhysics& e)
	{
		Ref<Scenes::Scene> scene = Scenes::SceneService::GetActiveScene();
		if (!scene) { return false; }
		ECS::Entity entity = scene->GetEntityByUUID(e.GetEntityID());
		if (!entity) { return false; }
		Math::vec3 translation = e.GetTranslation();
		Math::vec2 linearVelocity = e.GetLinearVelocity();
		entity.GetComponent<ECS::TransformComponent>().Translation = translation;

		if (entity.HasComponent<ECS::Rigidbody2DComponent>())
		{
			ECS::Rigidbody2DComponent& rb2d = entity.GetComponent<ECS::Rigidbody2DComponent>();
			b2Body* body = (b2Body*)rb2d.RuntimeBody;
			KG_ASSERT(body);
			body->SetTransform({ translation.x, translation.y }, body->GetAngle());
			body->SetLinearVelocity(b2Vec2(linearVelocity.x, linearVelocity.y));
		}

		return false;
	}

	void EngineService::OnSkipUpdate(Events::SkipUpdateEvent event)
	{
		s_ActiveEngine->m_Accumulator -= event.GetSkipCount() * k_ConstantFrameTime;
	}

	void EngineService::OnAddExtraUpdate(Events::AddExtraUpdateEvent event)
	{
		s_ActiveEngine->m_Accumulator += event.GetExtraUpdateCount() * k_ConstantFrameTime;
	}

	bool EngineService::OnCleanUpTimers(Events::CleanUpTimersEvent& e)
	{
		UNREFERENCED_PARAMETER(e);

		Utility::AsyncBusyTimer::CleanUpClosedTimers();
		return false;
	}

	bool EngineService::OnAddTickGeneratorUsage(Events::AddTickGeneratorUsage& e)
	{
		AppTickService::AddNewGenerator(e.GetDelayMilliseconds());
		return false;
	}

	bool EngineService::OnRemoveTickGeneratorUsage(Events::RemoveTickGeneratorUsage& e)
	{
		AppTickService::RemoveGenerator(e.GetDelayMilliseconds());
		return false;
	}

	bool EngineService::OnAppTickEvent(Events::AppTickEvent& e)
	{
		Network::ClientService::SubmitToNetworkEventQueue(CreateRef<Events::AppTickEvent>(e));
		return false;
	}

	void EngineService::RegisterWindowOnEventCallback()
	{
		s_ActiveEngine->m_Window->SetEventCallback(EngineService::OnEvent);
	}

	void EngineService::RegisterAppTickOnEventCallback()
	{
		AppTickService::SetAppTickEventCallback(EngineService::OnEvent);
	}

	void EngineService::RegisterCollisionEventListener(Physics::ContactListener& contactListener)
	{
		contactListener.SetEventCallback(EngineService::OnEvent);
	}

	void EngineService::SubmitToMainThread(const std::function<void()>& function)
	{
		s_ActiveEngine->m_WorkQueue.SubmitFunction(function);
	}

	void EngineService::SubmitToEventQueue(Ref<Events::Event> e)
	{
		s_ActiveEngine->m_EventQueue.SubmitEvent(e);
	}

	void EngineService::SubmitApplicationCloseEvent()
	{
		EngineService::SubmitToMainThread([&]()
		{
			Events::ApplicationCloseEvent event{};
			Events::EventCallbackFn eventCallback = EngineService::GetActiveWindow().GetEventCallback();
			eventCallback(&event);
		});
	}

	void EngineService::ProcessFunctionQueue()
	{
		KG_PROFILE_FUNCTION();
		s_ActiveEngine->m_WorkQueue.ProcessQueue();
	}

	void EngineService::ProcessEventQueue()
	{
		KG_PROFILE_FUNCTION();
		s_ActiveEngine->m_EventQueue.ProcessQueue();
	}

}
