#pragma once


#include "Core.h"

#include "Kargono/LayerStack.h"
#include "Kargono/Window.h"
#include "Kargono/Events/Event.h"
#include "Kargono/Events/ApplicationEvent.h"
#include "Kargono/ImGui/ImGuiLayer.h"
#include "Kargono/Renderer/Shader.h"
#include "Renderer/Buffer.h"
#include "Renderer/VertexArray.h"
#include "Kargono/Renderer/OrthographicCamera.h"


namespace Kargono
{
/// @brief The main application class for the game engine
	class KG_API Application
	{
/// @brief Public section of the application class
	public:
/// @brief Default constructor of the application class
		Application();
/// @brief Default destructor of the application class
		virtual ~Application();
/// @brief Function to run the application
		void Run();

/// @brief Function to handle events in the application
		void OnEvent(Event& e);

/// @brief Function to push a layer onto the layer stack
		void PushLayer(Layer* layer);
/// @brief Function to push an overlay onto the layer stack
		void PushOverlay(Layer* layer);

/// @brief Function to get the instance of the application
		inline static Application& Get() { return *s_Instance; }
/// @brief Function to get the application's window
		inline Window& GetWindow() { return *m_Window; }
	private:
/// @brief Function to handle window close events
		bool OnWindowClose(WindowCloseEvent& e);

/// @brief Unique pointer to the window of the application
		std::unique_ptr<Window> m_Window;
/// @brief Pointer to the ImGui layer of the application
		ImGuiLayer* m_ImGuiLayer;
/// @brief Boolean to control whether the application is running or not
		bool m_Running = true;
/// @brief Layer stack to manage the layers of the application
		LayerStack m_LayerStack;

/// @brief Shared pointer to the shader used in the application
		std::shared_ptr<Shader> m_Shader;
/// @brief Shared pointer to the blue shader used in the application
		std::shared_ptr<Shader> m_BlueShader;
/// @brief Shared pointer to the vertex array used in the application
		std::shared_ptr<VertexArray> m_VertexArray;

/// @brief Orthographic camera used in the application
		OrthographicCamera m_Camera;


/// @brief Shared pointer to the square vertex array used in the application
		std::shared_ptr<VertexArray> m_SquareVA;
	private:
		static Application* s_Instance;
	};

	// To be defined in client
	Application* CreateApplication();
}

