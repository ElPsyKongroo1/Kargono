#include "kgpch.h"

#include "Modules/Core/Engine.h"

#include "Kargono/Core/AppTick.h"
#include "Kargono/Utility/Time.h"
#include "Modules/Physics2D/Physics2D.h"
#include "Modules/Rendering/RenderingService.h"
#include "Kargono/Core/Profiler.h"
#include "Kargono/Utility/Timers.h"
#include "Modules/Events/NetworkingEvent.h"
#include "Modules/ECS/Entity.h"
#include "Kargono/Scenes/Scene.h"
#include "Modules/Physics2D/Physics2D.h"
#include "Modules/Network/Client.h"
#include "Modules/Scripting/ScriptService.h"


namespace Kargono
{
	float EngineThread::GetInApplicationTime() const
	{
		return (float)m_RunTimer.GetUpdateCount()* m_RunTimer.GetConstantFrameTimeFloat();
	}

	void EngineThread::UpdateAppStartTime()
	{
		m_EngineStartTime = Utility::Time::GetTime();
	}

	bool EngineThread::Init(Application* currentApp, Window* currentWindow)
	{
		KG_ASSERT(currentApp);
		KG_ASSERT(currentWindow);

		i_EngineApp = currentApp;
		i_EngineWindow = currentWindow;

		m_EventQueue.Init(KG_BIND_CLASS_FN(OnEvent));

		m_Active = true;

		return true;
	}

	bool EngineThread::Terminate(bool withinThread)
	{
		if (withinThread)
		{
			ClearThreadResources();
		}
		else
		{
			m_WorkQueue.SubmitFunction([this]()
			{
				ClearThreadResources();
			});
		}
		
		return true;
	}

	void EngineThread::ClearThreadResources()
	{
		i_EngineApp = nullptr;
		i_EngineWindow = nullptr;

		m_Active = false;
	}

	void EngineThread::RunThread()
	{
		if (!m_Active)
		{
			return;
		}

		constexpr float k_ConstantFrameTimeFloat{ 1.0f / 60.0f };
		constexpr uint64_t k_ConstantFrameTimeUInt{ 1'000 * 1'000 * 1'000 / 60 };
		KG_INFO("Starting Run Function");

		m_RunTimer.SetConstantFrameTime(k_ConstantFrameTimeUInt);
		m_RunTimer.InitializeTimer();

		while (m_Active)
		{
			if (m_RunTimer.CheckForSingleUpdate())
			{
				KG_PROFILE_FRAME_DESC("Main Thread");

				AppTickService::OnUpdate(k_ConstantFrameTimeFloat);
				Utility::PassiveTimer::OnUpdate(k_ConstantFrameTimeFloat);

				m_WorkQueue.ProcessQueue();

				i_EngineApp->OnUpdate(k_ConstantFrameTimeFloat);
				
				m_EventQueue.ProcessQueue();
				i_EngineWindow->OnUpdate();
			}
		}
		KG_INFO("Ending Run Function");
	}

	void EngineThread::EndThread()
	{
		m_WorkQueue.SubmitFunction([this]() 
		{
			m_Active = false;
		});
	}

	void EngineThread::OnEvent(Events::Event* e)
	{
		bool handled = false;
		// Process Input Events
		if (e->IsInCategory(Events::Input))
		{
			if (i_EngineApp)
			{
				i_EngineApp->OnInputEvent(e);
			}
			return;
		}

		// Process Physics Events
		if (e->IsInCategory(Events::Physics))
		{
			if (i_EngineApp)
			{
				i_EngineApp->OnPhysicsEvent(e);
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
			if (i_EngineApp)
			{
				i_EngineApp->OnNetworkEvent(e);
			}
			return;
		}

		if (e->IsInCategory(Events::Scene))
		{
			if (i_EngineApp)
			{
				i_EngineApp->OnSceneEvent(e);
			}
			return;
		}

		if (e->IsInCategory(Events::Asset))
		{
			if (i_EngineApp)
			{
				i_EngineApp->OnAssetEvent(e);
			}
			return;
		}

		// Process Application Events
		if (e->IsInCategory(Events::Application))
		{
			switch (e->GetEventType())
			{
			case Events::EventType::LogEvent:
				if (i_EngineApp)
				{
					handled = i_EngineApp->OnLogEvent(e);
				}
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

			if (i_EngineApp)
			{
				i_EngineApp->OnApplicationEvent(e);
			}

			return;
		}

		if (e->IsInCategory(Events::Editor))
		{
			if (i_EngineApp)
			{
				i_EngineApp->OnEditorEvent(e);
			}
			return;
		}
	}

	bool EngineThread::OnWindowClose(Events::WindowCloseEvent& /*event*/)
	{
		EndThread();
		return true;
	}

	bool EngineThread::OnWindowResize(Events::WindowResizeEvent& event)
	{
		if (event.GetWidth() == 0 || event.GetHeight() == 0)
		{
			return false;
		}

		Rendering::RenderingService::OnWindowResize(event.GetWidth(), event.GetHeight());
		return false;
	}

	bool EngineThread::OnUpdateEntityLocation(Events::UpdateEntityLocation& e)
	{
		Ref<Scenes::Scene> scene = Scenes::SceneService::GetActiveScene();
		if (!scene) { return false; }
		ECS::Entity entity = scene->GetEntityByUUID(e.GetEntityID());
		if (!entity) { return false; }
		Math::vec3 translation = e.GetTranslation();
		entity.GetComponent<ECS::TransformComponent>().Translation = translation;

		if (entity.HasComponent<ECS::Rigidbody2DComponent>())
		{
			ECS::Rigidbody2DComponent& rb2d = entity.GetComponent<ECS::Rigidbody2DComponent>();
			b2Body* body = (b2Body*)rb2d.RuntimeBody;
			body->SetTransform({ translation.x, translation.y }, body->GetAngle());
		}
		return false;
	}

	bool EngineThread::OnUpdateEntityPhysics(Events::UpdateEntityPhysics& e)
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

	void EngineThread::OnSkipUpdate(Events::SkipUpdateEvent event)
	{
		m_RunTimer.SkipUpdates(event.GetSkipCount());
	}

	void EngineThread::OnAddExtraUpdate(Events::AddExtraUpdateEvent event)
	{
		m_RunTimer.AddUpdates(event.GetExtraUpdateCount());
	}

	bool EngineThread::OnCleanUpTimers(Events::CleanUpTimersEvent& e)
	{
		UNREFERENCED_PARAMETER(e);

		Utility::AsyncBusyTimer::CleanUpClosedTimers();
		return false;
	}

	bool EngineThread::OnAddTickGeneratorUsage(Events::AddTickGeneratorUsage& e)
	{
		AppTickService::AddNewGenerator(e.GetDelayMilliseconds());
		return false;
	}

	bool EngineThread::OnRemoveTickGeneratorUsage(Events::RemoveTickGeneratorUsage& e)
	{
		AppTickService::RemoveGenerator(e.GetDelayMilliseconds());
		return false;
	}

	bool EngineThread::OnAppTickEvent(Events::AppTickEvent& e)
	{
		Network::ClientService::SubmitToNetworkEventQueue(CreateRef<Events::AppTickEvent>(e));
		return false;
	}

	void EngineThread::OnApplicationCloseEvent(Events::ApplicationCloseEvent& e)
	{
		Events::EventCallbackFn eventCallback = i_EngineWindow->GetEventCallback();
		eventCallback(&e);
	}

	void EngineThread::SubmitFunction(const std::function<void()> workFunction)
	{
		m_WorkQueue.SubmitFunction(workFunction);
	}

	void EngineThread::SubmitEvent(Ref<Events::Event> event)
	{
		m_EventQueue.SubmitEvent(event);
	}
	bool Engine::Init(const EngineConfig& config, Application* app)
	{
		KG_ASSERT(!m_Active);

		// Initialize data
		m_Config = config;
		m_CurrentApp = app;

		// Initialize main window
		m_Window = Window::Create(WindowProps(config.m_ExecutableName,
			config.m_DefaultWindowDimensions.x,
			config.m_DefaultWindowDimensions.y));
		KG_VERIFY(m_Window, "Created Engine Window");

		// Register callbacks
		RegisterWindowOnEventCallback();
		RegisterAppTickOnEventCallback();

		// Initialize engine thread
		if (!m_Thread.Init(app, m_Window.get()))
		{
			KG_WARN("Engine Init Failed. Main thread init failed");
			return false;
		}

		// Initialize current app (editor, runtime, server, etc...)
		if (!app->Init())
		{
			KG_WARN("Engine Init Failed. Application creation failed");
			return false;
		}

		m_Active = true;
		KG_VERIFY(m_Active, "Engine Initialized");
		return true;
	}
	bool Engine::Terminate()
	{
		if (!m_Active)
		{
			KG_WARN("Attempt to terminate engine service when s_ActiveEngine is already closed");
			return true;
		}

		if (m_CurrentApp)
		{
			m_CurrentApp->Terminate();
			delete m_CurrentApp;
			m_CurrentApp = nullptr;
		}
		KG_VERIFY(!m_CurrentApp, "Application Terminated");

		if (m_Window)
		{
			m_Window.reset();
		}

		m_Active = false;
		KG_VERIFY(!m_Active, "Active Engine Terminated");
		return true;
	}
	void Engine::RegisterWindowOnEventCallback()
	{
		EngineThread* threadPtr{ &m_Thread };
		m_Window->SetEventCallback(KG_BIND_CLASS_FN_EXPLICIT(threadPtr, OnEvent));
	}
	void Engine::RegisterAppTickOnEventCallback()
	{
		EngineThread* threadPtr{ &m_Thread };
		AppTickService::SetAppTickEventCallback(KG_BIND_CLASS_FN_EXPLICIT(threadPtr, OnEvent));
	}
	void Engine::RegisterCollisionEventListener(Physics::ContactListener& contactListener)
	{
		EngineThread* threadPtr{ &m_Thread };
		contactListener.SetEventCallback(KG_BIND_CLASS_FN_EXPLICIT(threadPtr, OnEvent));
	}
}
