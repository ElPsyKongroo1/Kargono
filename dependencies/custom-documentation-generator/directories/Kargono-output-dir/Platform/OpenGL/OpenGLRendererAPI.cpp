#include "Kargono/kgpch.h"
#include "OpenGLRendererAPI.h"
#include <glad/glad.h>

namespace Kargono
{
/// @brief Sets the clear color for the OpenGL renderer
///
/// @param color The color to set as the clear color
///
/// @source	This function is defined in the OpenGLRendererAPI.cpp file
	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}
/// @brief Clears the color and depth buffers of the OpenGL renderer
///
/// @source	This function is defined in the OpenGLRendererAPI.cpp file
	void OpenGLRendererAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
/// @brief Draws the given indexed vertex array using OpenGL
///
/// @param vertexArray The vertex array to draw
///
/// @source	This function is defined in the OpenGLRendererAPI.cpp file
	void OpenGLRendererAPI::DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray)
	{
		glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
	}
}