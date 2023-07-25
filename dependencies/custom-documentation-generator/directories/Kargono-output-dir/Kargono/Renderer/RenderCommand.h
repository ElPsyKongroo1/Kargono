#pragma once
#include "RendererAPI.h"

namespace Kargono
{
/// @brief Class for managing rendering commands in the game engine
	class RenderCommand
	{
	public:

/// @brief Sets the clear color for the renderer
/// @param color The new clear color as a glm::vec4
		inline static void SetClearColor(const glm::vec4& color)
		{
			s_RendererAPI->SetClearColor(color);
		};
/// @brief Clears the renderer's target buffer
		inline static void Clear() { s_RendererAPI->Clear(); }

/// @brief Draws geometry using the specified vertex array
/// @param vertexArray The vertex array containing the geometry to be drawn
		inline static void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray)
		{
			s_RendererAPI->DrawIndexed(vertexArray);
		}
	private:
		static RendererAPI* s_RendererAPI;
	};
}
