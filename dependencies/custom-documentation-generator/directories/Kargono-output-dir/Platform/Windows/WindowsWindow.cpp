#include "Kargono/kgpch.h"
#include "WindowsWindow.h"
#include "Kargono/Log.h"

#include "Kargono/Events/ApplicationEvent.h"
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Events/MouseEvent.h"

namespace Kargono 
{
/// @brief Static boolean variable to track whether GLFW is initialized or not.
	static bool s_GLFWInitialized = false;

/// @brief Error callback function for GLFW. Prints out the error and description.
	static void GLFWErrorCallback(int error, const char* description)
	{
		KG_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

/// @brief Create function to create a new window
	Window* Window::Create(const WindowProps& props)
	{
		return new WindowsWindow(props);
	}

/// @brief Constructor for the WindowsWindow class.
	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		Init(props);
	}
/// @brief Destructor for the WindowsWindow class.
	WindowsWindow::~WindowsWindow()
	{
	}

/// @brief Initialize function for the WindowsWindow class.
	void WindowsWindow::Init(const WindowProps& props)
	{
		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;

		KG_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);

		if (!s_GLFWInitialized)
		{
			int success = glfwInit();
			KG_CORE_ASSERT(success, "Could not initialize GLFW");
			glfwSetErrorCallback(GLFWErrorCallback);
			s_GLFWInitialized = true;
		}

		m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);
		glfwMakeContextCurrent(m_Window);
		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(true);

		// Set GLFW callbacks
/// @brief GLFW callback function for window size events.
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) 
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				data.Width = width;
				data.Height = height;
				WindowResizeEvent event(width, height);
				data.EventCallback(event);
			
			});

/// @brief GLFW callback function for window close events.
		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) 
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				WindowCloseEvent event;
				data.EventCallback(event);
			});

/// @brief GLFW callback function for key events.
		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				switch (action)
				{
					case GLFW_PRESS:
					{
						KeyPressedEvent event(key, 0);
						data.EventCallback(event);
					}
						break;
					case GLFW_RELEASE:
					{
						KeyReleasedEvent event(key);
						data.EventCallback(event);
					}
						break;
					case GLFW_REPEAT:
					{
						KeyPressedEvent event(key, 1);
						data.EventCallback(event);
					}
						break;
					}
			});

/// @brief GLFW callback function for mouse button events.
		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				switch (action)
					{
					case GLFW_PRESS:
					{
						MouseButtonPressedEvent event(button);
						data.EventCallback(event);
					}
						break;
					case GLFW_RELEASE:
					{
						MouseButtonReleasedEvent event(button);
						data.EventCallback(event);
					}
						break;
					}
			});

/// @brief GLFW callback function for scroll events.
		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset) 
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				MouseScrolledEvent event((float)xOffset, (float)yOffset);
				data.EventCallback(event);
			});
/// @brief GLFW callback function for cursor position events.
		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos) 
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				MouseMovedEvent event((float)xPos, (float)yPos);
				data.EventCallback(event);
			});

	}

/// @brief Shutdown function for the WindowsWindow class.
	void WindowsWindow::Shutdown()
	{
		glfwDestroyWindow(m_Window);
	}

/// @brief Update function for the WindowsWindow class.
	void WindowsWindow::OnUpdate() 
	{
		glfwPollEvents();
		glfwSwapBuffers(m_Window);
	}

/// @brief SetVSync function for the WindowsWindow class.
	void WindowsWindow::SetVSync(bool enabled)
	{
		if (enabled)	{ glfwSwapInterval(1); }
		else			{ glfwSwapInterval(0); }
		m_Data.VSync = enabled;
	}

/// @brief IsVSync function for the WindowsWindow class.
	bool WindowsWindow::IsVSync() const
	{
		return m_Data.VSync;
	}
}
