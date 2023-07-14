#pragma once


#include "Core.h"

#include "Kargono/LayerStack.h"
#include "Kargono/Window.h"
#include "Kargono/Events/Event.h"
#include "Kargono/Events/ApplicationEvent.h"


namespace Kargono
{
/// @class Application
	class KG_API Application
	{
/// @public
	public:
		Application();
/// @brief Destructor for the Application class
		virtual ~Application();
/// @brief Function to run the application
		void Run();

/// @brief Function to handle events in the application
		void OnEvent(Event& e);

/// @brief Function to push a layer onto the layer stack
		void PushLayer(Layer* layer);
/// @brief Function to push an overlay onto the layer stack
		void PushOverlay(Layer* layer);

/// @brief Static function to get the instance of the Application
		inline static Application& Get() { return *s_Instance; }
/// @brief Function to get the application window
		inline Window& GetWindow() { return *m_Window; }
	private:
/// @brief Function called when the window is closed
		bool OnWindowClose(WindowCloseEvent& e);

/// @brief The unique pointer to the Window object
		std::unique_ptr<Window> m_Window;
/// @brief Flag to indicate if the application is running
		bool m_Running = true;
/// @brief The layer stack object
		LayerStack m_LayerStack;
	private:
/// @brief The static instance of the Application class
		static Application* s_Instance;
	};

	// To be defined in client
/// @brief Function to create the application object
	Application* CreateApplication();
}

