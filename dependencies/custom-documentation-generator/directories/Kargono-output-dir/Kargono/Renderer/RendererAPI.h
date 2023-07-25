#pragma once

#include <glm/glm.hpp>

#include "VertexArray.h"

namespace Kargono
{
	

/// @class RendererAPI
	class RendererAPI
	{
	public:
/// @enum API
		enum class API
		{
			None = 0, OpenGL = 1
		};
	public:
/// @brief Set the clear color for the rendering context.
		virtual void SetClearColor(const glm::vec4& color) = 0;
/// @brief Clear the rendering context.
		virtual void Clear() = 0;

/// @brief Draw the vertices of a vertex array with indices.
		virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) = 0;

/// @brief Get the current rendering API.
		inline static API GetAPI() { return s_API; }
	private:
		static API s_API;
	};

}
