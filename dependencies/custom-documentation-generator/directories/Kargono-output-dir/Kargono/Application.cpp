#include "Kargono/kgpch.h"
#include "Application.h"

#include "Log.h"

#include <glad/glad.h>

#include "Input.h"


namespace Kargono
{
/// @brief Macro to bind an event function to the Application class.
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

/// @brief Static instance of the Application class.
	Application* Application::s_Instance = nullptr;

/// @brief Constructor for the Application class.
	Application::Application()
	{
/// @brief Asserts that the application instance does not already exist.
		KG_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;
/// @brief Creates a unique pointer to a Window object.
		m_Window = std::unique_ptr<Window>(Window::Create());
/// @brief Sets the callback function for the window events.
		m_Window->SetEventCallback(BIND_EVENT_FN(OnEvent));

/// @brief Creates a new ImGuiLayer object.
		m_ImGuiLayer = new ImGuiLayer();
/// @brief Adds the ImGuiLayer as an overlay to the application layers stack.
		PushOverlay(m_ImGuiLayer);

/// @brief Generates a vertex array object.
		glGenVertexArrays(1, &m_VertexArray);
/// @brief Binds the vertex array object.
/// @brief Binds the vertex array object for rendering.
		glBindVertexArray(m_VertexArray);


		float vertices[3 * 3] =
		{
			//  X      Y      Z
				-0.5f, -0.5f, 0.0f,
				0.5f, -0.5f, 0.0f,
				0.0f, 0.5f, 0.0f
		};
/// @brief Resets the vertex buffer with the vertex positions.
		m_VertexBuffer.reset(VertexBuffer::Create(vertices, sizeof(vertices)));

		BufferLayout layout = {
			{ShaderDataType::Float3,  "a_Position"},
			
		}
		m_VertexBuffer->SetLayout(layout);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);


/// @brief Defines an array of indices for the mesh.
		unsigned int indices[3] = { 0, 1, 2 };
/// @brief Resets the index buffer with the mesh indices.
		m_IndexBuffer.reset(IndexBuffer::Create(indices, sizeof(indices) / sizeof(uint32_t)));

/// @brief Defines the source code of the vertex shader.
		std::string vertexSrc = R"(
			#version 450 core
		
			layout(location = 0) in vec3 a_Position;
			out vec3 v_Position;
	
			void main ()
			{
				v_Position = a_Position;
				gl_Position = vec4(a_Position, 1.0f);
			}
		)";

/// @brief Defines the source code of the fragment shader.
		std::string fragmentSrc = R"(
			#version 450 core
		
			layout(location = 0) out vec4 color;
			in vec3 v_Position;
	
			void main ()
			{
				color = vec4(v_Position * 0.5 + 0.5, 1.0f);
			}
		)";


/// @brief Resets the shader with the vertex and fragment shaders.
		m_Shader.reset(new Shader(vertexSrc, fragmentSrc));
	}
/// @brief Destructor for the Application class.
	Application::~Application()
	{
	
	}

/// @brief Pushes a layer to the layers stack and attaches it.
	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

/// @brief Pushes an overlay layer to the layers stack and attaches it.
	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}


/// @brief Handles an event by dispatching it to the relevant functions.
	void Application::OnEvent(Event& e) 
	{

/// @brief Creates an event dispatcher for the given event.
		EventDispatcher dispatcher(e);
/// @brief Dispatches the WindowCloseEvent to the OnWindowClose function.
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(OnWindowClose));

/// @brief Iterates through the layers stack in reverse order.
		for (auto location = m_LayerStack.end(); location != m_LayerStack.begin();)
		{
/// @brief Calls the OnEvent function of each layer.
			(*--location)->OnEvent(e);
/// @brief Checks if the event has been handled and breaks the loop if true.
			if (e.Handled)
				break;
		}
	}

	

/// @brief Runs the main application loop.
	void Application::Run()
	{
		while (m_Running)
		{
/// @brief Clears the color buffer with a specified color.
			glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
/// @brief Clears the color buffer.
			glClear(GL_COLOR_BUFFER_BIT);

/// @brief Binds the shader for rendering.
			m_Shader->Bind();
			glBindVertexArray(m_VertexArray);
/// @brief Draws the elements of the mesh.
			glDrawElements(GL_TRIANGLES, m_IndexBuffer->GetCount(), GL_UNSIGNED_INT, nullptr);

/// @brief Iterates through the layers stack.
			for (Layer* layer : m_LayerStack)
			{
/// @brief Calls the OnUpdate function of each layer.
				layer->OnUpdate();
			}
/// @brief Begins the ImGui rendering.
			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
			{
/// @brief Calls the OnImGuiRender function of each layer.
				layer->OnImGuiRender();
			}
/// @brief Ends the ImGui rendering.
			m_ImGuiLayer->End();

/// @brief Updates the window.
			m_Window->OnUpdate();
		}
	}

/// @brief Handles the window close event.
	bool Application::OnWindowClose(WindowCloseEvent& e) 
	{
/// @brief Sets the running state of the application to false.
		m_Running = false;
/// @brief Returns true to indicate that the event was handled.
		return true;
	}

}