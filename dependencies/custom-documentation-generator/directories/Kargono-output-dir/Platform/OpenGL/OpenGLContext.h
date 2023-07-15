
#pragma once

#include "Kargono/Renderer/GraphicsContext.h"

/// @brief Forward declaration of GLFWwindow struct
struct GLFWwindow;

/// @brief Namespace for the Kargono game engine
namespace Kargono
{
/// @class OpenGLContext : public GraphicsContext
	class OpenGLContext : public GraphicsContext
	{

	public:
		OpenGLContext(GLFWwindow* windowHandle);
	public:

/// @brief Initializes the OpenGL context
		virtual void Init() override;
/// @brief Swaps the front and back buffers of the GLFW window
		virtual void SwapBuffers() override;

	private:
/// @brief Pointer to the GLFW window handle
		GLFWwindow* m_WindowHandle;
	};

}