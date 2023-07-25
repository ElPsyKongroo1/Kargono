#pragma once
#include "RendererAPI.h"

namespace Kargono
{
	
/// @class Renderer
/// @brief This class is responsible for rendering functionality in the game engine
	class Renderer
	{
	public:
/// @brief Begins a new render scene
		static void BeginScene();
/// @brief Ends the current render scene
		static void EndScene();

/// @brief Submits a vertex array for rendering
		static void Submit(const std::shared_ptr<VertexArray>& vertexArray);
/// @brief Retrieves the current API used for rendering
/// @return The current renderer API
		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	};
}
