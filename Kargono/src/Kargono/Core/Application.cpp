#include "kgpch.h"

#include "Kargono/Core/Application.h"
#include "Kargono/Utils/PlatformUtils.h"
#include "Kargono/Renderer/Renderer.h"
#include "Kargono/Scripting/ScriptEngine.h"

namespace Kargono
{
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application(const ApplicationSpecification& specification)
		: m_Specification(specification)
		
	{

		KG_CORE_ASSERT(!s_Instance, "Application already exists!")
		s_Instance = this;
		// Set working directory here
		if (!m_Specification.WorkingDirectory.empty())
			std::filesystem::current_path(m_Specification.WorkingDirectory);

		m_Window = Window::Create(WindowProps(m_Specification.Name));
		m_Window->SetEventCallback(KG_BIND_EVENT_FN(Application::OnEvent));

		Renderer::Init();
		

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	Application::~Application()
	{

		ScriptEngine::Shutdown();
		Renderer::Shutdown();
		
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


	void Application::OnEvent(Event& e) 
	{
		

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(KG_BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(KG_BIND_EVENT_FN(Application::OnWindowResize));

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

	void Application::SubmitToMainThread(const std::function<void()>& function)
	{
		std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

		m_MainThreadQueue.emplace_back(function);
	}

	void Application::Run()
	{
		

		while (m_Running)
		{
			float time = Time::GetTime();
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			ExecuteMainThreadQueue();

			if (!m_Minimized)
			{
				{
					for (Layer* layer : m_LayerStack)
					{
						layer->OnUpdate(timestep);
					}
				}
				m_ImGuiLayer->Begin();
				{
					for (Layer* layer : m_LayerStack)
					{
						layer->OnImGuiRender();
					}
				}
				m_ImGuiLayer->End();
			}

			m_Window->OnUpdate();
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e) 
	{
		m_Running = false;
		return true;
	}
	bool Application::OnWindowResize(WindowResizeEvent& e)
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
		for (auto& func : m_MainThreadQueue) { func(); }
		m_MainThreadQueue.clear();
	}

}
