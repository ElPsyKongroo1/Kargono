#include "Kargono/kgpch.h"
#include "Application.h"
#include "Events/ApplicationEvent.h"
#include "Log.h"


namespace Kargono
{

/// @brief Constructor for the Application class
	Application::Application()
	{
		m_Window = std::unique_ptr<Window>(Window::Create());
	}
/// @brief Destructor for the Application class
	Application::~Application()
	{
	
	}

/// @brief Function to start running the application
	void Application::Run()
	{
		while (m_Running)
		{

			m_Window->OnUpdate();
		}
	}

}