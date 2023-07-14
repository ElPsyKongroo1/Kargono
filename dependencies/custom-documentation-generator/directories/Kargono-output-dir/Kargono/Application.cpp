#include "Kargono/kgpch.h"
#include "Application.h"

#include "Log.h"

#include <glad/glad.h>


/// @namespace Kargono
namespace Kargono
{
/// @brief Macro for binding an event to a member function of the Application class
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

/// @brief Static pointer to the instance of the Application class
	Application* Application::s_Instance = nullptr;

/// @brief Constructor for the Application class
	Application::Application()
	{
		KG_CORE_ASSERT(!s_Instance, "Applicatino already exists!");
		s_Instance = this;
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
		layer->OnAttach();
	}

/// @brief Pushes an overlay onto the layer stack
	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}


/// @brief Handles events from the window
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

	

/// @brief Runs the game engine
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

/// @brief Handles the window close event
	bool Application::OnWindowClose(WindowCloseEvent& e) 
	{
		m_Running = false;
		return true;
	}

}