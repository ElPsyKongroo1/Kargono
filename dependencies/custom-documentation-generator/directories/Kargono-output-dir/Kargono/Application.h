#pragma once

#include "Core.h"
#include "Events/Event.h"
#include "Window.h"
namespace Kargono
{
/// @class Application
/// @brief Class representing the main application
/// @details This class is responsible for creating and managing the application window and running the game loop
	class KG_API Application
	{
	public:
/// @brief Default constructor for the Application class
		Application();
/// @brief Virtual destructor for the Application class
		virtual ~Application();
/// @brief Runs the main game loop
/// @details This function continuously updates and renders the game until the application is closed
		void Run();
	private:
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
	};

	// To be defined in client
	Application* CreateApplication();
}

