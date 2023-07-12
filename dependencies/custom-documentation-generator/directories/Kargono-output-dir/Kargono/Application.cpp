#include "Kargono/kgpch.h"
#include "Application.h"

#include "Log.h"

#include <GLFW/glfw3.h>


namespace Kargono
{
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

/// @brief Constructor for the Application class
	Application::Application()
	{
		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));
	}
/// @brief Destructor for the Application class
	Application::~Application()
	{
	
	}

/// @brief Pushes a layer onto the layer stack
	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
	}

/// @brief Pushes an overlay onto the layer stack
	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
	}


/// @brief Called when an event occurs in the application
	void Application::OnEvent(Event& e) 
	{

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));

		for (auto location = m_LayerStack.end(); location != m_LayerStack.begin();)
		{
			(*--location)->OnEvent(e);
			if (e.Handled)
				break;
		}
	}

	

/// @brief The main game loop of the application
	void Application::Run()
	{
		while (m_Running)
		{
			glClearColor(1, 0, 1, 1);
			glClear(GL_COLOR_BUFFER_BIT);

			for (Layer* layer : m_LayerStack)
			{
				layer->OnUpdate();
			}
			m_Window->OnUpdate();
		}
	}

/// @brief Called when the window is closed
	bool Application::OnWindowClose(WindowCloseEvent& e) 
	{
		m_Running = false;
		return true;
	}

}