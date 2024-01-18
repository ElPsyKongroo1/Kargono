#include "kgpch.h"

#include "Kargono/Core/Application.h"

#include "AppTick.h"
#include "Kargono/Utils/PlatformUtils.h"
#include "Kargono/Script/ScriptEngine.h"
#include "Kargono/Physics/Physics2D.h"
#include "Kargono/Renderer/RenderCommand.h"
#include "Kargono/Renderer/Renderer.h"
#include "Kargono/Core/Profiler.h"

namespace Kargono
{
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationSpecification& specification)
		: m_Specification(specification)
		
	{
		// Ensure Application is a Singleton
		KG_CORE_ASSERT(!s_Instance, "Application already exists!")
		s_Instance = this;

		m_Window = Window::Create(WindowProps(m_Specification.Name, m_Specification.Width, m_Specification.Height));
		m_Window->SetEventCallback(KG_BIND_EVENT_FN(Application::OnEvent));
		AppTickEngine::SetAppTickCallback(KG_BIND_EVENT_FN(Application::OnEvent));

		Script::ScriptEngine::Init();
		Audio::AudioEngine::Init();

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

	void Application::Run()
	{
		constexpr float constantFrameTime = 1.0f / 60.0f;
		static float lastFrameTime = 0.0f;
		static float accumulator = 0.0f;

		while (m_Running)
		{
			float currentTime = Utility::Time::GetTime();
			Timestep timestep = currentTime - lastFrameTime;
			lastFrameTime = currentTime;
			accumulator += timestep;
			if (accumulator < constantFrameTime) { continue; }
			accumulator -= constantFrameTime;
			if (accumulator > constantFrameTime) { accumulator = 0; }

			{
				KG_PROFILE_FRAME("Main Thread");

				AppTickEngine::UpdateGenerators(constantFrameTime);

				ExecuteMainThreadQueue();
				if (!m_Minimized)
				{
					{
						for (Layer* layer : m_LayerStack)
						{
							layer->OnUpdate(constantFrameTime);
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

	void Application::ExecuteMainThreadQueue()
	{
		KG_PROFILE_FUNCTION();

		for (auto& func : m_MainThreadQueue) { func(); }
		m_MainThreadQueue.clear();
	}

}
