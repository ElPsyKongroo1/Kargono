#include "Kargono/kgpch.h"
#include "Kargono/Core/Application.h"

#include "Kargono/Core/Log.h"

#include "Kargono/Core/Input.h"
#include "GLFW/glfw3.h"
#include "Kargono/Renderer/Renderer.h"
#include "Kargono/Renderer/Renderer.h"
#include "Kargono/Renderer/RenderCommand.h"



namespace Kargono
{
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application()
		
	{
		KG_PROFILE_FUNCTION();

		KG_CORE_ASSERT(!s_Instance, "Application already exists!")
		s_Instance = this;
		m_Window = Window::Create();
		m_Window->SetEventCallback(KG_BIND_EVENT_FN(Application::OnEvent));

		Renderer::Init();

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	Application::~Application()
	{
		KG_PROFILE_FUNCTION();

		Renderer::Shutdown();
	}

	void Application::PushLayer(Layer* layer)
	{
		KG_PROFILE_FUNCTION();

		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		KG_PROFILE_FUNCTION();

		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}


	void Application::OnEvent(Event& e) 
	{
		KG_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(KG_BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(KG_BIND_EVENT_FN(Application::OnWindowResize));

		for (auto location = m_LayerStack.rbegin(); location != m_LayerStack.rend(); ++ location)
		{
			(*location)->OnEvent(e);
			if (e.Handled)
				break;
		}
	}

	

	void Application::Run()
	{
		KG_PROFILE_FUNCTION();

		while (m_Running)
		{
			KG_PROFILE_SCOPE("RunLoop");

			float time = (float)glfwGetTime(); // Platform::GetTime()
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			if (!m_Minimized)
			{
				{
					KG_PROFILE_SCOPE("LayerStack OnUpdate");

					for (Layer* layer : m_LayerStack)
					{
						layer->OnUpdate(timestep);
					}
				}
				m_ImGuiLayer->Begin();
				{
					KG_PROFILE_SCOPE("LayerStack OnImGuiRender");
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
		KG_PROFILE_FUNCTION();

		if(e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}

		m_Minimized = false;
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;
	}

}
