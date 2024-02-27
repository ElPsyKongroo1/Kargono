#include "kgpch.h"

#include "Kargono/Core/Application.h"

#include "AppTick.h"

#include "Kargono/Utils/PlatformUtils.h"
#include "Kargono/Script/ScriptEngine.h"
#include "Kargono/Physics/Physics2D.h"
#include "Kargono/Renderer/RenderCommand.h"
#include "Kargono/Renderer/Renderer.h"
#include "Kargono/Core/Profiler.h"
#include "Kargono/Core/Timers.h"
#include "Kargono/Events/NetworkingEvent.h"
#include "Kargono/Scene/Entity.h"
#include "Kargono/Scene/Scene.h"
#include "Kargono/Physics/Physics2D.h"
#include "Kargono/Network/Client.h"

#include <windows.h>

#include "Kargono/Scripting/Scripting.h"

namespace Kargono
{
//#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	std::chrono::nanoseconds k_ConstantFrameTime { 1'000 * 1'000 * 1'000 / 60 };
	Timestep k_ConstantFrameTimeStep { 1.0f / 60.0f };

	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationSpecification& specification)
		: m_Specification(specification)
		
	{
		// Ensure Application is a Singleton
		KG_ASSERT(!s_Instance, "Application already exists!")
		s_Instance = this;

		m_Window = Window::Create(WindowProps(m_Specification.Name, m_Specification.Width, m_Specification.Height));
		m_Window->SetEventCallback(KG_BIND_EVENT_FN(Application::OnEvent));
		AppTickEngine::SetAppTickCallback(KG_BIND_EVENT_FN(Application::OnEvent));

	}

	Application::~Application()
	{

		for (Layer* layer : m_LayerStack.GetLayers())
		{
			if (layer)
			{
				layer->OnDetach();
				delete layer;
				layer = nullptr;
			}
		}

		Script::ScriptEngine::Shutdown();
		Scripting::ScriptCore::Terminate();
		Audio::AudioEngine::Terminate();
		// TODO: Add Renderer Shutdown!
		
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}


	void Application::OnEvent(Events::Event& e) 
	{
		Events::EventDispatcher dispatcher(e);
		dispatcher.Dispatch<Events::WindowCloseEvent>(KG_BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<Events::WindowResizeEvent>(KG_BIND_EVENT_FN(Application::OnWindowResize));
		dispatcher.Dispatch<Events::CleanUpTimersEvent>(KG_BIND_EVENT_FN(Application::OnCleanUpTimers));
		dispatcher.Dispatch<Events::AddTickGeneratorUsage>(KG_BIND_EVENT_FN(Application::OnAddTickGeneratorUsage));
		dispatcher.Dispatch<Events::RemoveTickGeneratorUsage>(KG_BIND_EVENT_FN(Application::OnRemoveTickGeneratorUsage));
		dispatcher.Dispatch<Events::AppTickEvent>(KG_BIND_EVENT_FN(Application::OnAppTickEvent));

		dispatcher.Dispatch<Events::UpdateEntityLocation>(KG_BIND_EVENT_FN(Application::OnUpdateEntityLocation));
		dispatcher.Dispatch<Events::UpdateEntityPhysics>(KG_BIND_EVENT_FN(Application::OnUpdateEntityPhysics));

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++ it)
		{
			if (e.Handled) { break; }
			(*it)->OnEvent(e);
		}
	}

	

	void Application::Close()
	{
		m_Running = false;
	}

	void Application::SetAppStartTime()
	{
		m_AppStartTime = Utility::Time::GetTime();
	}

	void Application::SubmitToMainThread(const std::function<void()>& function)
	{
		std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

		m_MainThreadQueue.emplace_back(function);
	}

	void Application::SubmitToEventQueue(Ref<Events::Event> e)
	{
		std::scoped_lock<std::mutex> lock(m_EventQueueMutex);

		m_EventQueue.emplace_back(e);
	}

	void Application::OnSkipUpdate(Events::SkipUpdateEvent event)
	{
		m_Accumulator -= event.GetSkipCount() * k_ConstantFrameTime;
	}

	void Application::OnAddExtraUpdate(Events::AddExtraUpdateEvent event)
	{
		m_Accumulator += event.GetExtraUpdateCount() * k_ConstantFrameTime;
	}

	void Application::Run()
	{
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
					{
						for (Layer* layer : m_LayerStack)
						{
							layer->OnUpdate(k_ConstantFrameTimeStep);
						}
					}
					if (m_ImGuiLayer)
					{
						m_ImGuiLayer->Begin();
						{
							for (Layer* layer : m_LayerStack)
							{
								layer->OnImGuiRender();
							}
						}
						m_ImGuiLayer->End();
					}
				
				}
				ProcessEventQueue();
				m_Window->OnUpdate();
			}
		}
	}

	bool Application::OnWindowClose(Events::WindowCloseEvent& e) 
	{
		m_Running = false;
		return true;
	}
	bool Application::OnWindowResize(Events::WindowResizeEvent& e)
	{

		if(e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}

		m_Minimized = false;
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;
	}

	bool Application::OnCleanUpTimers(Events::CleanUpTimersEvent& e)
	{
		Timers::AsyncBusyTimer::CleanUpClosedTimers();
		return false;
	}

	bool Application::OnAddTickGeneratorUsage(Events::AddTickGeneratorUsage& e)
	{
		AppTickEngine::AddGeneratorUsage(e.GetDelayMilliseconds());
		return false;
	}

	bool Application::OnRemoveTickGeneratorUsage(Events::RemoveTickGeneratorUsage& e)
	{
		AppTickEngine::RemoveGeneratorUsage(e.GetDelayMilliseconds());
		return false;
	}

	bool Application::OnAppTickEvent(Events::AppTickEvent& e)
	{
		auto client = Network::Client::GetActiveClient();
		if (client)
		{
			client->SubmitToEventQueue(CreateRef<Events::AppTickEvent>(e));
		}
		return false;
	}

	bool Application::OnUpdateEntityLocation(Events::UpdateEntityLocation& e)
	{
		Scene* scene = Script::ScriptEngine::GetSceneContext();
		if (!scene) { return false; }
		Entity entity = scene->GetEntityByUUID(e.GetEntityID());
		if (!entity) { return false; }
		Math::vec3 translation = e.GetTranslation();
		entity.GetComponent<TransformComponent>().Translation = translation;

		if (entity.HasComponent<Rigidbody2DComponent>())
		{
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
			b2Body* body = (b2Body*)rb2d.RuntimeBody;
			body->SetTransform({ translation.x, translation.y }, body->GetAngle());
		}
		return false;
	}

	bool Application::OnUpdateEntityPhysics(Events::UpdateEntityPhysics& e)
	{
		Scene* scene = Script::ScriptEngine::GetSceneContext();
		if (!scene) { return false; }
		Entity entity = scene->GetEntityByUUID(e.GetEntityID());
		if (!entity) { return false; }
		Math::vec3 translation = e.GetTranslation();
		Math::vec2 linearVelocity = e.GetLinearVelocity();
		entity.GetComponent<TransformComponent>().Translation = translation;

		if (entity.HasComponent<Rigidbody2DComponent>())
		{
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
			b2Body* body = (b2Body*)rb2d.RuntimeBody;
			body->SetTransform({ translation.x, translation.y }, body->GetAngle());
			body->SetLinearVelocity(b2Vec2(linearVelocity.x, linearVelocity.y));
		}
		
		return false;
	}

	

	void Application::ExecuteMainThreadQueue()
	{
		KG_PROFILE_FUNCTION();
		std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

		for (auto& func : m_MainThreadQueue) { func(); }
		m_MainThreadQueue.clear();
	}

	void Application::ProcessEventQueue()
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
