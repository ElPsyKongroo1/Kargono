/// @brief Include guard to prevent multiple inclusion of this file
#pragma once
/// @brief Include the RendererAPI header file
#include "Kargono/Renderer/RendererAPI.h"
/// @brief Include the VertexArray header file
#include "Kargono/Renderer/VertexArray.h"
/// @brief Namespace for the Kargono game engine
namespace Kargono {

/// @class OpenGLRendererAPI
/// @brief Class representing the OpenGL implementation of the RendererAPI
	class OpenGLRendererAPI : public Kargono::RendererAPI
	{
/// @public Access specifier for the following members
	public:
/// @brief Set the clear color for the renderer
/// @param color The color to set, specified using the glm::vec4 class
		virtual void SetClearColor(const glm::vec4& color) override;
/// @brief Clear the rendering context
		virtual void Clear() override;

/// @brief Draw the specified vertex array
/// @param vertexArray The vertex array to draw, specified using shared_ptr<VertexArray> class
		virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) override;
	};
}
