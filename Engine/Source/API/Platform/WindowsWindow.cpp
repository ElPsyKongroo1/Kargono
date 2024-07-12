#include "kgpch.h"

#include "Kargono/Events/ApplicationEvent.h"
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Events/MouseEvent.h"
#include "Kargono/Rendering/RenderingService.h"
#include "Kargono/Projects/Project.h"

#include "API/Platform/WindowsWindow.h"
#include "API/ImageProcessing/stbAPI.h"
#include "API/Platform/gladAPI.h"

#ifdef KG_PLATFORM_WINDOWS

namespace API::Utility
{
	static void GLFWErrorCallback(int error, const char* description)
	{
		KG_ERROR("GLFW Error ({0}): {1}", error, description);
	}
}

namespace API::Platform
{
	static uint8_t s_GLFWWindowCount = 0;

	WindowsWindow::WindowsWindow(const Kargono::WindowProps& props)
	{
		// Update Class Data
		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;
		m_Data.ViewportWidth = props.Width;
		m_Data.ViewportHeight = props.Height;
		m_Window = nullptr;

	}
	WindowsWindow::~WindowsWindow()
	{
		Shutdown();
	}

	void WindowsWindow::Init(const Kargono::WindowProps& props, const std::filesystem::path& logoPath)
	{
		// Update Class Data
		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;
		m_Data.ViewportWidth = props.Width;
		m_Data.ViewportHeight = props.Height;
		Init(logoPath);
	}

	void WindowsWindow::Init(const std::filesystem::path& logoPath)
	{
		// Ensure Only One Window Instance is active
		if (s_GLFWWindowCount > 0)
		{
			KG_ERROR("Attempt to initialize another glfwWindow.");
			return;
		}
		// Start Initializing GLFW
		KG_INFO("Creating window {0} ({1}, {2})", m_Data.Title, m_Data.Width, m_Data.Height);
		
		KG_INFO("Initializing GLFW");
		int success = glfwInit();
		KG_ASSERT(success, "Could not initialize GLFW");
		glfwSetErrorCallback(Utility::GLFWErrorCallback);

		// Create New Window Through GLFW
		#if defined(KG_DEBUG) && defined(KG_RENDERER_OPENGL)
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
		#endif
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		m_Window = glfwCreateWindow((int)m_Data.Width, (int)m_Data.Height, m_Data.Title.c_str(), nullptr, nullptr);
		++s_GLFWWindowCount;

		// Make Context Current and Load GLAD (GLAD obtains function pointers for OpenGL functions from GPU Drivers)
		glfwMakeContextCurrent(m_Window);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		KG_ASSERT(status, "Failed to initialize Glad!");

		KG_INFO("OpenGL Info:");
		KG_INFO("\tVendor: {0}", (const char*)glGetString(GL_VENDOR));
		KG_INFO("\tRenderer: {0}", (const char*)glGetString(GL_RENDERER));
		KG_INFO("\tVersion: {0}", (const char*)glGetString(GL_VERSION));
		KG_ASSERT(GLVersion.major > m_Data.VersionMajor || (GLVersion.major == m_Data.VersionMajor && GLVersion.minor >= m_Data.VersionMinor), "Kargono requires at least OpenGL version 4.5!");

		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(false);

		// Set GLFW callbacks
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) 
			{
				KG_PROFILE_FUNCTION("GLFW Resize Event");

				WindowsWindowData& data = *(WindowsWindowData*)glfwGetWindowUserPointer(window);

				data.Width = width;
				data.Height = height;
				Kargono::Events::WindowResizeEvent event(width, height);
				data.EventCallback(event);
			
			});

		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) 
			{
				KG_PROFILE_FUNCTION("GLFW Close Event");
				WindowsWindowData& data = *(WindowsWindowData*)glfwGetWindowUserPointer(window);
				Kargono::Events::WindowCloseEvent event;
				data.EventCallback(event);
			});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				KG_PROFILE_FUNCTION("GLFW Keyboard Event");

				WindowsWindowData& data = *(WindowsWindowData*)glfwGetWindowUserPointer(window);
				switch (action)
				{
					case GLFW_PRESS:
					{
						Kargono::Events::KeyPressedEvent event(key, false);
						data.EventCallback(event);
					}
						break;
					case GLFW_RELEASE:
					{
						Kargono::Events::KeyReleasedEvent event(key);
						data.EventCallback(event);
					}
						break;
					case GLFW_REPEAT:
					{
						Kargono::Events::KeyPressedEvent event(key, true);
						data.EventCallback(event);
					}
						break;
					}
			});

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode)
			{
				KG_PROFILE_FUNCTION("GLFW Typing Event");

				WindowsWindowData& data = *(WindowsWindowData*)glfwGetWindowUserPointer(window);
				Kargono::Events::KeyTypedEvent event(keycode);
				data.EventCallback(event);

			});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
			{
				KG_PROFILE_FUNCTION("GLFW Mouse Button Event");

				WindowsWindowData& data = *(WindowsWindowData*)glfwGetWindowUserPointer(window);

				switch (action)
					{
					case GLFW_PRESS:
					{
						Kargono::Events::MouseButtonPressedEvent event(button);
						data.EventCallback(event);
					}
						break;
					case GLFW_RELEASE:
					{
						Kargono::Events::MouseButtonReleasedEvent event(button);
						data.EventCallback(event);
					}
						break;
					}
			});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset) 
			{
				KG_PROFILE_FUNCTION("GLFW Scroll Event");

				WindowsWindowData& data = *(WindowsWindowData*)glfwGetWindowUserPointer(window);

				Kargono::Events::MouseScrolledEvent event((float)xOffset, (float)yOffset);
				data.EventCallback(event);
			});
		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos) 
			{
				KG_PROFILE_FUNCTION("GLFW Move Cursor Event");

				WindowsWindowData& data = *(WindowsWindowData*)glfwGetWindowUserPointer(window);

				Kargono::Events::MouseMovedEvent event((float)xPos, (float)yPos);
				data.EventCallback(event);
			});

		// Add App Logo
		if (!std::filesystem::exists(logoPath)) { KG_ERROR("Path to Application Logo is invalid!"); return; }
		GLFWimage images[1];
		images[0].pixels = stbi_load(logoPath.string().c_str(), &images[0].width, &images[0].height, 0, 4); //rgba channels 
		glfwSetWindowIcon(m_Window, 1, images);
		stbi_image_free(images[0].pixels);

	}

	void WindowsWindow::Shutdown()
	{
		glfwDestroyWindow(m_Window);
		--s_GLFWWindowCount;

		if (s_GLFWWindowCount == 0)
		{
			KG_INFO("All GLFW Windows have been closed and GLFW has terminated!");
			glfwTerminate();
		}
	}

	void WindowsWindow::SwapBuffers()
	{
		KG_PROFILE_FUNCTION();
		glfwSwapBuffers(m_Window);
	}


	void WindowsWindow::OnUpdate() 
	{
		KG_PROFILE_FUNCTION();
		{
			KG_PROFILE_FUNCTION("GLFW Polling");
			glfwPollEvents();
		}
		
		SwapBuffers();
	}

	void WindowsWindow::SetVSync(bool enabled)
	{
		if (enabled)	{ glfwSwapInterval(1); }
		else			{ glfwSwapInterval(0); }
		m_Data.VSync = enabled;
	}
	void WindowsWindow::SetFullscreen(bool enabled)
	{
		if (enabled)
		{
			GLFWmonitor* monitor = glfwGetPrimaryMonitor();
			if (!monitor)
			{
				KG_WARN("No primary monitor is available! Failed to set monitor to fullscreen.");
				return;
			}
			glfwSetWindowMonitor(m_Window, glfwGetPrimaryMonitor(), 0, 0, m_Data.Width, m_Data.Height, GLFW_DONT_CARE);
		}
		else
		{
			glfwSetWindowMonitor(m_Window, nullptr, 0, 0, m_Data.Width, m_Data.Height, GLFW_DONT_CARE);
		}
	}
	void WindowsWindow::SetResizable(bool resizable)
	{
		glfwSetWindowAttrib(m_Window, GLFW_RESIZABLE, resizable ? GLFW_TRUE : GLFW_FALSE);
	}
	void WindowsWindow::CenterWindow()
	{
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		if (!monitor)
		{
			KG_WARN("No primary monitor could be located. Failed to center window.");
			return;
		}
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwSetWindowPos(m_Window, (mode->width - m_Data.Width) / 2, (mode->height - m_Data.Height) / 2);
	}
	glm::vec2 WindowsWindow::GetMonitorDimensions()
	{
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		if (!monitor)
		{
			KG_WARN("Could not locate primary monitor! Returning default monitor dimensions.");
			return glm::vec2(400.0f, 400.0f);
		}
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		return glm::vec2(static_cast<float>(mode->width), static_cast<float>(mode->height));
	}
	void WindowsWindow::ResizeWindow(glm::vec2 newWindowSize)
	{
		glfwSetWindowSize(m_Window, static_cast<int>(newWindowSize.x), static_cast<int>(newWindowSize.y));

		m_Data.Width = static_cast<uint32_t>(newWindowSize.x);
		m_Data.Height = static_cast<uint32_t>(newWindowSize.y);

		if (!Kargono::Projects::ProjectService::GetActiveIsFullscreen())
		{
			CenterWindow();
		}

		// Event thrown to ensure resize updates viewport
		Kargono::Events::WindowResizeEvent event(static_cast<uint32_t>(newWindowSize.x), static_cast<uint32_t>(newWindowSize.y));
		m_Data.EventCallback(event);

	}
	void WindowsWindow::ToggleMaximized()
	{
		if (glfwGetWindowAttrib(m_Window, GLFW_MAXIMIZED))
		{
			glfwRestoreWindow(m_Window);
		}
		else
		{
			glfwMaximizeWindow(m_Window);
		}
	}
	void WindowsWindow::SetMouseCursorVisible(bool choice)
	{
		auto cursorVisibility = choice ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
		glfwSetInputMode(m_Window, GLFW_CURSOR, cursorVisibility);
	}
	void WindowsWindow::SetVisible(bool visible)
	{
		if (visible)
		{
			glfwShowWindow(m_Window);
		}
		else
		{
			glfwHideWindow(m_Window);
		}
	}
}

#endif
