#include "kgpch.h"

#include "Kargono/Events/ApplicationEvent.h"
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Events/MouseEvent.h"
#include "Kargono/Renderer/Renderer.h"
#include "API/Windows/WindowsWindow.h"

#include "stb_image.h"
#include "glad/glad.h"


namespace Kargono 
{
	static uint8_t s_GLFWWindowCount = 0;

	static void GLFWErrorCallback(int error, const char* description)
	{
		KG_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		Init(props);
	}
	WindowsWindow::~WindowsWindow()
	{
		Shutdown();
	}

	void WindowsWindow::Init(const WindowProps& props)
	{
		if (s_GLFWWindowCount > 0)
		{
			KG_CORE_ASSERT(false, "Attempt to initialize another glfwWindow.");
			return;
		}

		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;

		KG_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);
		
		KG_CORE_INFO("Initializing GLFW");
		int success = glfwInit();
		KG_CORE_ASSERT(success, "Could not initialize GLFW");
		glfwSetErrorCallback(GLFWErrorCallback);
	
		#if defined(KG_DEBUG)
		if (Renderer::GetAPI() == RendererAPI::API::OpenGL) { glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE); }
		#endif
		m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);
		++s_GLFWWindowCount;

		glfwMakeContextCurrent(m_Window);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		KG_CORE_ASSERT(status, "Failed to initialize Glad!");

		KG_CORE_INFO("OpenGL Info:");
		KG_CORE_INFO("\tVendor: {0}", (const char*)glGetString(GL_VENDOR));
		KG_CORE_INFO("\tRenderer: {0}", (const char*)glGetString(GL_RENDERER));
		KG_CORE_INFO("\tVersion: {0}", (const char*)glGetString(GL_VERSION));
		KG_CORE_ASSERT(GLVersion.major > m_Data.VersionMajor || (GLVersion.major == m_Data.VersionMajor && GLVersion.minor >= m_Data.VersionMinor), "Kargono requires at least OpenGL version 4.5!");

		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(true);

		// Set GLFW callbacks
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) 
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				data.Width = width;
				data.Height = height;
				WindowResizeEvent event(width, height);
				data.EventCallback(event);
			
			});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) 
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				WindowCloseEvent event;
				data.EventCallback(event);
			});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				switch (action)
				{
					case GLFW_PRESS:
					{
						KeyPressedEvent event(key, false);
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
						KeyPressedEvent event(key, true);
						data.EventCallback(event);
					}
						break;
					}
			});

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				KeyTypedEvent event(keycode);
				data.EventCallback(event);

			});

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

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset) 
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				MouseScrolledEvent event((float)xOffset, (float)yOffset);
				data.EventCallback(event);
			});
		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos) 
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				MouseMovedEvent event((float)xPos, (float)yPos);
				data.EventCallback(event);
			});

		// Add App Logo
		std::filesystem::path logoLocation = "resources/icons/app_logo.png";
		if (!std::filesystem::exists(logoLocation)) { KG_CORE_ERROR("Path to Application Logo is invalid!"); return; }
		GLFWimage images[1];
		images[0].pixels = stbi_load(logoLocation.string().c_str(), &images[0].width, &images[0].height, 0, 4); //rgba channels 
		glfwSetWindowIcon(m_Window, 1, images);
		stbi_image_free(images[0].pixels);

	}

	void WindowsWindow::Shutdown()
	{
		

		glfwDestroyWindow(m_Window);
		--s_GLFWWindowCount;

		if (s_GLFWWindowCount == 0)
		{
			KG_CORE_INFO("GLFW Terminate was Called!");
			glfwTerminate();
		}
	}

	void WindowsWindow::SwapBuffers()
	{
		glfwSwapBuffers(m_Window);
	}


	void WindowsWindow::OnUpdate() 
	{
		glfwPollEvents();
		SwapBuffers();
	}

	void WindowsWindow::SetVSync(bool enabled)
	{
		

		if (enabled)	{ glfwSwapInterval(1); }
		else			{ glfwSwapInterval(0); }
		m_Data.VSync = enabled;
	}

	bool WindowsWindow::IsVSync() const
	{
		return m_Data.VSync;
	}
	void WindowsWindow::SetMouseCursorVisible(bool choice)
	{
		auto cursorVisibility = choice ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
		glfwSetInputMode(m_Window, GLFW_CURSOR, cursorVisibility);
	}
}
