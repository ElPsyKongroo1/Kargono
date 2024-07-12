#include "kgpch.h"

#include "Kargono/Core/Engine.h"

#include "Kargono/Core/AppTick.h"
#include "Kargono/Utility/Time.h"
#include "Kargono/Physics/Physics2D.h"
#include "Kargono/Rendering/RenderingService.h"
#include "Kargono/Core/Profiler.h"
#include "Kargono/Utility/Timers.h"
#include "Kargono/Events/NetworkingEvent.h"
#include "Kargono/Scenes/Entity.h"
#include "Kargono/Scenes/Scene.h"
#include "Kargono/Physics/Physics2D.h"
#include "Kargono/Network/Client.h"
#include "Kargono/Scripting/Scripting.h"


namespace Kargono
{
//#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	std::chrono::nanoseconds k_ConstantFrameTime { 1'000 * 1'000 * 1'000 / 60 };
	Timestep k_ConstantFrameTimeStep { 1.0f / 60.0f };

	Engine* EngineService::s_ActiveEngine = nullptr;
	

	void Engine::UpdateAppStartTime()
	{
		m_AppStartTime = Utility::Time::GetTime();
	}
	
	void EngineService::Init(const EngineSpec& specification, Application* app)
	{
		// Ensure Engine is a Singleton
		KG_ASSERT(!s_ActiveEngine, "Application already exists!")
		s_ActiveEngine = new Engine();
		s_ActiveEngine->m_Specification = specification;
		s_ActiveEngine->m_CurrentApp = app;
		s_ActiveEngine->m_Window = Window::Create(WindowProps(s_ActiveEngine->m_Specification.Name, 
			s_ActiveEngine->m_Specification.DefaultWindowWidth, 
			s_ActiveEngine->m_Specification.DefaultWindowHeight));
		KG_VERIFY(s_ActiveEngine->m_Window, "Window Init");
		RegisterWindowOnEventCallback();
		RegisterAppTickOnEventCallback();
		app->OnAttach();
		KG_VERIFY(s_ActiveEngine, "Engine Initialized");
	}

	void EngineService::Terminate()
	{
		if (s_ActiveEngine->m_CurrentApp)
		{
			s_ActiveEngine->m_CurrentApp->OnDetach();
			delete s_ActiveEngine->m_CurrentApp;
			s_ActiveEngine->m_CurrentApp = nullptr;
		}

		KG_VERIFY(!(s_ActiveEngine->m_CurrentApp), "Application Terminated");
	}

	void EngineService::Run()
	{
		KG_INFO("Starting Run Function");
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
				KG_PROFILE_FRAME("Main Thread");

				s_ActiveEngine->m_UpdateCount++;
				AppTickService::OnUpdate(k_ConstantFrameTimeStep);

				ExecuteMainThreadQueue();
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

	void EngineService::OnEvent(Events::Event& e)
	{
		Events::EventDispatcher dispatcher(e);
		dispatcher.Dispatch<Events::WindowCloseEvent>(EngineService::OnWindowClose);
		dispatcher.Dispatch<Events::WindowResizeEvent>(EngineService::OnWindowResize);
		dispatcher.Dispatch<Events::CleanUpTimersEvent>(EngineService::OnCleanUpTimers);
		dispatcher.Dispatch<Events::AddTickGeneratorUsage>(EngineService::OnAddTickGeneratorUsage);
		dispatcher.Dispatch<Events::RemoveTickGeneratorUsage>(EngineService::OnRemoveTickGeneratorUsage);
		dispatcher.Dispatch<Events::AppTickEvent>(EngineService::OnAppTickEvent);

		dispatcher.Dispatch<Events::UpdateEntityLocation>(EngineService::OnUpdateEntityLocation);
		dispatcher.Dispatch<Events::UpdateEntityPhysics>(EngineService::OnUpdateEntityPhysics);

		if (e.Handled)
		{
			return;
		}
		if (s_ActiveEngine->m_CurrentApp)
		{
			s_ActiveEngine->m_CurrentApp->OnEvent(e);
		}
	}

	bool EngineService::OnWindowClose(Events::WindowCloseEvent& e)
	{
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
		Scenes::Entity entity = scene->GetEntityByUUID(e.GetEntityID());
		if (!entity) { return false; }
		Math::vec3 translation = e.GetTranslation();
		entity.GetComponent<Scenes::TransformComponent>().Translation = translation;

		if (entity.HasComponent<Scenes::Rigidbody2DComponent>())
		{
			auto& rb2d = entity.GetComponent<Scenes::Rigidbody2DComponent>();
			b2Body* body = (b2Body*)rb2d.RuntimeBody;
			body->SetTransform({ translation.x, translation.y }, body->GetAngle());
		}
		return false;
	}

	bool EngineService::OnUpdateEntityPhysics(Events::UpdateEntityPhysics& e)
	{
		Ref<Scenes::Scene> scene = Scenes::SceneService::GetActiveScene();
		if (!scene) { return false; }
		Scenes::Entity entity = scene->GetEntityByUUID(e.GetEntityID());
		if (!entity) { return false; }
		Math::vec3 translation = e.GetTranslation();
		Math::vec2 linearVelocity = e.GetLinearVelocity();
		entity.GetComponent<Scenes::TransformComponent>().Translation = translation;

		if (entity.HasComponent<Scenes::Rigidbody2DComponent>())
		{
			auto& rb2d = entity.GetComponent<Scenes::Rigidbody2DComponent>();
			b2Body* body = (b2Body*)rb2d.RuntimeBody;
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
		auto client = Network::Client::GetActiveClient();
		if (client)
		{
			client->SubmitToEventQueue(CreateRef<Events::AppTickEvent>(e));
		}
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
		std::scoped_lock<std::mutex> lock(s_ActiveEngine->m_MainThreadQueueMutex);

		s_ActiveEngine->m_MainThreadQueue.emplace_back(function);
	}

	void EngineService::SubmitToEventQueue(Ref<Events::Event> e)
	{
		std::scoped_lock<std::mutex> lock(s_ActiveEngine->m_EventQueueMutex);

		s_ActiveEngine->m_EventQueue.emplace_back(e);
	}

	void EngineService::SubmitApplicationCloseEvent()
	{
		EngineService::SubmitToMainThread([&]()
		{
			Events::ApplicationCloseEvent event{};
			Events::EventCallbackFn eventCallback = EngineService::GetActiveWindow().GetEventCallback();
			eventCallback(event);
		});
	}

	void EngineService::ExecuteMainThreadQueue()
	{
		KG_PROFILE_FUNCTION();
		std::scoped_lock<std::mutex> lock(s_ActiveEngine->m_MainThreadQueueMutex);

		for (auto& func : s_ActiveEngine->m_MainThreadQueue) 
		{ 
			func(); 
		}
		s_ActiveEngine->m_MainThreadQueue.clear();
	}

	void EngineService::ProcessEventQueue()
	{
		KG_PROFILE_FUNCTION();

		std::scoped_lock<std::mutex> lock(s_ActiveEngine->m_EventQueueMutex);

		for (auto& event : s_ActiveEngine->m_EventQueue)
		{
			OnEvent(*event);
		}
		s_ActiveEngine->m_EventQueue.clear();
	}

}
