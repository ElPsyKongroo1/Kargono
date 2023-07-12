#pragma once


#include "Core.h"

#include "Kargono/LayerStack.h"
#include "Kargono/Window.h"
#include "Kargono/Events/Event.h"
#include "Kargono/Events/ApplicationEvent.h"


namespace Kargono
{
	class KG_API Application
	{
	public:
/// @brief Constructor for the Application class
		Application();
/// @brief Virtual destructor for the Application class
		virtual ~Application();
/// @brief Start running the application
		void Run();

/// @brief Event handler for events that occur within the application
		void OnEvent(Event& e);

/// @brief Add a layer to the layer stack
		void PushLayer(Layer* layer);
/// @brief Add an overlay layer to the layer stack
		void PushOverlay(Layer* layer);
	private:
/// @brief Event handler for the window close event
		bool OnWindowClose(WindowCloseEvent& e);

		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
		LayerStack m_LayerStack;
	};

	// To be defined in client
	Application* CreateApplication();
}

