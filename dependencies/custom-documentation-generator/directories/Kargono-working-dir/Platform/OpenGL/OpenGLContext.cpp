#include "Kargono/kgpch.h"
#include "OpenGLContext.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "Kargono/Core.h"
#include "Kargono/Log.h"


namespace Kargono
{
	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle) : m_WindowHandle{windowHandle}
	{
		KG_CORE_ASSERT(windowHandle, "Window handle is null!");
	}
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
	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_WindowHandle);
	}
}
