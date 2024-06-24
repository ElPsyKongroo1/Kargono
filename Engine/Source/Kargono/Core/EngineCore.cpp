#include "kgpch.h"

#include "Kargono/Core/EngineCore.h"
#include "Kargono/Core/AppTick.h"
#include "Kargono/Utility/Time.h"
#include "Kargono/Script/ScriptEngine.h"
#include "Kargono/Physics/Physics2D.h"
#include "Kargono/Rendering/RenderingService.h"
#include "Kargono/Core/Profiler.h"
#include "Kargono/Core/Timers.h"
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

	EngineCore* EngineCore::s_CurrentEngineCore = nullptr;

	EngineCore::EngineCore(const AppSpec& specification, Application* app)
		: m_Specification(specification)
		
	{
		// Ensure Application is a Singleton
		KG_ASSERT(!s_CurrentEngineCore, "Application already exists!")
		s_CurrentEngineCore = this;
		m_CurrentApp = app;
		m_Window = Window::Create(WindowProps(m_Specification.Name, m_Specification.Width, m_Specification.Height));
		KG_VERIFY(m_Window, "Window Init");
		m_Window->SetEventCallback(KG_BIND_CLASS_FN(EngineCore::OnEvent));
		AppTickEngine::SetAppTickCallback(KG_BIND_CLASS_FN(EngineCore::OnEvent));
		app->OnAttach();
	}

	EngineCore::~EngineCore()
	{
		if (m_CurrentApp)
		{
			m_CurrentApp->OnDetach();
			delete m_CurrentApp;
			m_CurrentApp = nullptr;
		}

		Script::ScriptEngine::Shutdown();
		Scripting::ScriptService::Terminate();
		Audio::AudioService::Terminate();
		// TODO: Add Renderer Shutdown!

		KG_VERIFY(!m_CurrentApp, "Close App");
		
	}

	void EngineCore::OnEvent(Events::Event& e) 
	{
		Events::EventDispatcher dispatcher(e);
		dispatcher.Dispatch<Events::WindowCloseEvent>(KG_BIND_CLASS_FN(EngineCore::OnWindowClose));
		dispatcher.Dispatch<Events::WindowResizeEvent>(KG_BIND_CLASS_FN(EngineCore::OnWindowResize));
		dispatcher.Dispatch<Events::CleanUpTimersEvent>(KG_BIND_CLASS_FN(EngineCore::OnCleanUpTimers));
		dispatcher.Dispatch<Events::AddTickGeneratorUsage>(KG_BIND_CLASS_FN(EngineCore::OnAddTickGeneratorUsage));
		dispatcher.Dispatch<Events::RemoveTickGeneratorUsage>(KG_BIND_CLASS_FN(EngineCore::OnRemoveTickGeneratorUsage));
		dispatcher.Dispatch<Events::AppTickEvent>(KG_BIND_CLASS_FN(EngineCore::OnAppTickEvent));

		dispatcher.Dispatch<Events::UpdateEntityLocation>(KG_BIND_CLASS_FN(EngineCore::OnUpdateEntityLocation));
		dispatcher.Dispatch<Events::UpdateEntityPhysics>(KG_BIND_CLASS_FN(EngineCore::OnUpdateEntityPhysics));

		if (e.Handled)
		{
			return;
		}
		if (m_CurrentApp)
		{
			m_CurrentApp->OnEvent(e);
		}
	}

	

	void EngineCore::Close()
	{
		m_Running = false;
	}

	void EngineCore::SetAppStartTime()
	{
		m_AppStartTime = Utility::Time::GetTime();
	}

	void EngineCore::SubmitToMainThread(const std::function<void()>& function)
	{
		std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

		m_MainThreadQueue.emplace_back(function);
	}

	void EngineCore::SubmitToEventQueue(Ref<Events::Event> e)
	{
		std::scoped_lock<std::mutex> lock(m_EventQueueMutex);

		m_EventQueue.emplace_back(e);
	}

	void EngineCore::OnSkipUpdate(Events::SkipUpdateEvent event)
	{
		m_Accumulator -= event.GetSkipCount() * k_ConstantFrameTime;
	}

	void EngineCore::OnAddExtraUpdate(Events::AddExtraUpdateEvent event)
	{
		m_Accumulator += event.GetExtraUpdateCount() * k_ConstantFrameTime;
	}

	void EngineCore::Run()
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
				AppTickEngine::UpdateGenerators(k_ConstantFrameTimeStep);

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

	bool EngineCore::OnWindowClose(Events::WindowCloseEvent& e) 
	{
		m_Running = false;
		return true;
	}
	bool EngineCore::OnWindowResize(Events::WindowResizeEvent& e)
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

	bool EngineCore::OnCleanUpTimers(Events::CleanUpTimersEvent& e)
	{
		AsyncBusyTimer::CleanUpClosedTimers();
		return false;
	}

	bool EngineCore::OnAddTickGeneratorUsage(Events::AddTickGeneratorUsage& e)
	{
		AppTickEngine::AddGeneratorUsage(e.GetDelayMilliseconds());
		return false;
	}

	bool EngineCore::OnRemoveTickGeneratorUsage(Events::RemoveTickGeneratorUsage& e)
	{
		AppTickEngine::RemoveGeneratorUsage(e.GetDelayMilliseconds());
		return false;
	}

	bool EngineCore::OnAppTickEvent(Events::AppTickEvent& e)
	{
		auto client = Network::Client::GetActiveClient();
		if (client)
		{
			client->SubmitToEventQueue(CreateRef<Events::AppTickEvent>(e));
		}
		return false;
	}

	bool EngineCore::OnUpdateEntityLocation(Events::UpdateEntityLocation& e)
	{
		Scenes::Scene* scene = Script::ScriptEngine::GetSceneContext();
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

	bool EngineCore::OnUpdateEntityPhysics(Events::UpdateEntityPhysics& e)
	{
		Scenes::Scene* scene = Script::ScriptEngine::GetSceneContext();
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

	

	void EngineCore::ExecuteMainThreadQueue()
	{
		KG_PROFILE_FUNCTION();
		std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

		for (auto& func : m_MainThreadQueue) { func(); }
		m_MainThreadQueue.clear();
	}

	void EngineCore::ProcessEventQueue()
	{
		KG_PROFILE_FUNCTION();

		std::scoped_lock<std::mutex> lock(m_EventQueueMutex);

		for (auto& event : m_EventQueue)
		{
			OnEvent(*event);
		}
		m_EventQueue.clear();
	}

}
