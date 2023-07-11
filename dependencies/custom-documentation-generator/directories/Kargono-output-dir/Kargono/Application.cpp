/// @brief Include the precompiled header file
#include "Kargono/kgpch.h"
/// @brief Include the Application header file
#include "Application.h"
/// @brief Include the ApplicationEvent header file
#include "Events/ApplicationEvent.h"
/// @brief Include the Log header file
#include "Log.h"


namespace Kargono
{

/// @brief Constructor for the Application class
	Application::Application()
	{
/// @brief Create a unique_ptr for the Window object and assign the result of Window::Create()
		m_Window = std::unique_ptr<Window>(Window::Create());
	}
/// @brief Destructor for the Application class
	Application::~Application()
	{
	
	}

/// @brief Run the application
	void Application::Run()
	{
/// @brief Execute the loop while the application is running
		while (m_Running)
		{

/// @brief Update the window
			m_Window->OnUpdate();
		}
	}

}