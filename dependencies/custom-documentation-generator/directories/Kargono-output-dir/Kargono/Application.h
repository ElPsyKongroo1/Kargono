/// @brief Preprocessor directive to ensure header file is only included once
#pragma once


/// @brief Including the Core header file
#include "Core.h"

/// @brief Including the LayerStack header file from Kargono namespace
#include "Kargono/LayerStack.h"
/// @brief Including the Window header file from Kargono namespace
#include "Kargono/Window.h"
/// @brief Including the Event header file from Kargono namespace
#include "Kargono/Events/Event.h"
/// @brief Including the ApplicationEvent header file from Kargono namespace
#include "Kargono/Events/ApplicationEvent.h"
/// @brief Including the ImGuiLayer header file from Kargono namespace
#include "Kargono/ImGui/ImGuiLayer.h"
/// @brief Including the Shader header file from Kargono namespace
#include "Kargono/Renderer/Shader.h"
/// @brief Including the Buffer header file from Renderer namespace
#include "Renderer/Buffer.h"


namespace Kargono
{
/// @class Application
	class KG_API Application
	{
/// @brief Public access specifier
	public:
/// @brief Default constructor of the Application class
		Application();
/// @brief Virtual destructor of the Application class
		virtual ~Application();
/// @brief Function to run the application
		void Run();

/// @brief Event handler function
		void OnEvent(Event& e);

/// @brief Function to add a layer to the layer stack
		void PushLayer(Layer* layer);
/// @brief Function to add an overlay to the layer stack
		void PushOverlay(Layer* layer);

/// @brief Static function to get the instance of the Application class
		inline static Application& Get() { return *s_Instance; }
/// @brief Function to get the application window
		inline Window& GetWindow() { return *m_Window; }
/// @brief Private access specifier
/// @brief Private access specifier
	private:
/// @brief Event handler function for window close event
		bool OnWindowClose(WindowCloseEvent& e);

/// @brief Pointer to the Window instance
		std::unique_ptr<Window> m_Window;
/// @brief Pointer to the ImGuiLayer instance
		ImGuiLayer* m_ImGuiLayer;
/// @brief Boolean variable to indicate if the application is running
		bool m_Running = true;
/// @brief Instance of the LayerStack class
		LayerStack m_LayerStack;

/// @brief Unsigned integer variable to hold the vertex array ID
		unsigned int m_VertexArray;
/// @brief Pointer to the VertexBuffer instance
		std::unique_ptr<VertexBuffer> m_VertexBuffer;
/// @brief Pointer to the IndexBuffer instance
		std::unique_ptr<IndexBuffer> m_IndexBuffer;
/// @brief Pointer to the Shader instance
		std::unique_ptr<Shader> m_Shader;
	private:
/// @brief Pointer to the Application instance
		static Application* s_Instance;
/// @brief Closing the class declaration
	};

	// To be defined in client
/// @brief Declaration of external CreateApplication function
	Application* CreateApplication();
}

