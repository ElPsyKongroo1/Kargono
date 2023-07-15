#include "Kargono/kgpch.h"
#include "OpenGLContext.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "Kargono/Core.h"
#include "Kargono/Log.h"


/// @namespace Kargono
namespace Kargono
{
/// @brief Constructor for the OpenGLContext class
/// @param windowHandle - The handle to the GLFW window
	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle) : m_WindowHandle{windowHandle}
	{
		KG_CORE_ASSERT(windowHandle, "Window handle is null!");
	}
/// @brief Initializes the OpenGLContext class and sets the current rendering context to the GLFW window handle
/// @return void
	void OpenGLContext::Init()
	{
		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		KG_CORE_ASSERT(status, "Failed to initialize Glad!");

		KG_CORE_INFO("OpenGL Info:");
		KG_CORE_INFO("\tVendor: {0}", (const char*)glGetString(GL_VENDOR));
		KG_CORE_INFO("\tRenderer: {0}", (const char*)glGetString(GL_RENDERER));
		KG_CORE_INFO("\tVersion: {0}", (const char*)glGetString(GL_VERSION));
	}
/// @brief Swaps the front and back buffers of the GLFW window to update the screen
/// @return void
	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_WindowHandle);
	}
}
