#include "kgpch.h"

#include "Kargono/Core/EngineCore.h"
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

	Engine::Engine(const EngineSpec& specification, Application* app)
		: m_Specification(specification)
		
	{
		// Ensure Application is a Singleton
		KG_ASSERT(!EngineService::s_ActiveEngine, "Application already exists!")
		EngineService::s_ActiveEngine = this;
		m_CurrentApp = app;
		m_Window = Window::Create(WindowProps(m_Specification.Name, m_Specification.DefaultWindowWidth, m_Specification.DefaultWindowHeight));
		KG_VERIFY(m_Window, "Window Init");
		m_Window->SetEventCallback(KG_BIND_CLASS_FN(Engine::OnEvent));
		AppTickService::SetAppTickEventCallback(KG_BIND_CLASS_FN(Engine::OnEvent));
		app->OnAttach();
	}

	Engine::~Engine()
	{
		if (m_CurrentApp)
		{
			m_CurrentApp->OnDetach();
			delete m_CurrentApp;
			m_CurrentApp = nullptr;
		}

		KG_VERIFY(!m_CurrentApp, "Close App");
		
	}

	void Engine::OnEvent(Events::Event& e) 
	{
		Events::EventDispatcher dispatcher(e);
		dispatcher.Dispatch<Events::WindowCloseEvent>(KG_BIND_CLASS_FN(Engine::OnWindowClose));
		dispatcher.Dispatch<Events::WindowResizeEvent>(KG_BIND_CLASS_FN(Engine::OnWindowResize));
		dispatcher.Dispatch<Events::CleanUpTimersEvent>(KG_BIND_CLASS_FN(Engine::OnCleanUpTimers));
		dispatcher.Dispatch<Events::AddTickGeneratorUsage>(KG_BIND_CLASS_FN(Engine::OnAddTickGeneratorUsage));
		dispatcher.Dispatch<Events::RemoveTickGeneratorUsage>(KG_BIND_CLASS_FN(Engine::OnRemoveTickGeneratorUsage));
		dispatcher.Dispatch<Events::AppTickEvent>(KG_BIND_CLASS_FN(Engine::OnAppTickEvent));

		dispatcher.Dispatch<Events::UpdateEntityLocation>(KG_BIND_CLASS_FN(Engine::OnUpdateEntityLocation));
		dispatcher.Dispatch<Events::UpdateEntityPhysics>(KG_BIND_CLASS_FN(Engine::OnUpdateEntityPhysics));

		if (e.Handled)
		{
			return;
		}
		if (m_CurrentApp)
		{
			m_CurrentApp->OnEvent(e);
		}
	}

	

	void Engine::CloseEngine()
	{
		m_Running = false;
	}

	void Engine::SetAppStartTime()
	{
		m_AppStartTime = Utility::Time::GetTime();
	}


	void Engine::SubmitApplicationCloseEvent()
	{
		EngineService::SubmitToMainThread([&]()
		{
			Events::ApplicationCloseEvent event {};
			Events::EventCallbackFn eventCallback = EngineService::GetActiveWindow().GetEventCallback();
			eventCallback(event);
		});
	}

	void Engine::OnSkipUpdate(Events::SkipUpdateEvent event)
	{
		m_Accumulator -= event.GetSkipCount() * k_ConstantFrameTime;
	}

	void Engine::OnAddExtraUpdate(Events::AddExtraUpdateEvent event)
	{
		m_Accumulator += event.GetExtraUpdateCount() * k_ConstantFrameTime;
	}

	void Engine::RunOnUpdate()
	{
		KG_INFO("Starting Run Function");
		using namespace std::chrono_literals;

		std::chrono::time_point<std::chrono::high_resolution_clock> currentTime = std::chrono::high_resolution_clock::now();
		std::chrono::time_point<std::chrono::high_resolution_clock> lastCycleTime = currentTime;
		std::chrono::nanoseconds timestep{ 0 };

		while (m_Running)
		{
			currentTime = std::chrono::high_resolution_clock::now();
			timestep = currentTime - lastCycleTime;
			lastCycleTime = currentTime;
			m_Accumulator += timestep;
			if (m_Accumulator < k_ConstantFrameTime)
			{
				continue;
			}
			m_Accumulator -= k_ConstantFrameTime;

			{
				KG_PROFILE_FRAME("Main Thread");

				m_UpdateCount++;
				AppTickService::OnUpdate(k_ConstantFrameTimeStep);

				ExecuteMainThreadQueue();
				if (!m_Minimized)
				{
					m_CurrentApp->OnUpdate(k_ConstantFrameTimeStep);
				}
				ProcessEventQueue();
				m_Window->OnUpdate();
			}
		}
		KG_INFO("Ending Run Function");
	}

	bool Engine::OnWindowClose(Events::WindowCloseEvent& e) 
	{
		m_Running = false;
		return true;
	}
	bool Engine::OnWindowResize(Events::WindowResizeEvent& e)
	{

		if(e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}

		m_Minimized = false;
		Rendering::RenderingService::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;
	}

	bool Engine::OnCleanUpTimers(Events::CleanUpTimersEvent& e)
	{
		Utility::AsyncBusyTimer::CleanUpClosedTimers();
		return false;
	}

	bool Engine::OnAddTickGeneratorUsage(Events::AddTickGeneratorUsage& e)
	{
		AppTickService::AddNewGenerator(e.GetDelayMilliseconds());
		return false;
	}

	bool Engine::OnRemoveTickGeneratorUsage(Events::RemoveTickGeneratorUsage& e)
	{
		AppTickService::RemoveGenerator(e.GetDelayMilliseconds());
		return false;
	}

	bool Engine::OnAppTickEvent(Events::AppTickEvent& e)
	{
		auto client = Network::Client::GetActiveClient();
		if (client)
		{
			client->SubmitToEventQueue(CreateRef<Events::AppTickEvent>(e));
		}
		return false;
	}

	bool Engine::OnUpdateEntityLocation(Events::UpdateEntityLocation& e)
	{
		Ref<Scenes::Scene> scene = Scenes::Scene::GetActiveScene();
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

	bool Engine::OnUpdateEntityPhysics(Events::UpdateEntityPhysics& e)
	{
		Ref<Scenes::Scene> scene = Scenes::Scene::GetActiveScene();
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

	

	void Engine::ExecuteMainThreadQueue()
	{
		KG_PROFILE_FUNCTION();
		std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

		for (auto& func : m_MainThreadQueue) { func(); }
		m_MainThreadQueue.clear();
	}

	void Engine::ProcessEventQueue()
	{
		KG_PROFILE_FUNCTION();

		std::scoped_lock<std::mutex> lock(m_EventQueueMutex);

		for (auto& event : m_EventQueue)
		{
			OnEvent(*event);
		}
		m_EventQueue.clear();
	}

	void EngineService::Init()
	{
	}

	void EngineService::Terminate()
	{
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

}
