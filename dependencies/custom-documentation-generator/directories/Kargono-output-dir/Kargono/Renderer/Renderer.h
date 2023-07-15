#pragma once

namespace Kargono
{
/// @brief Enumeration defining different renderer APIs supported by the game engine
/// @brief Class representing the renderer in the game engine
	enum class RendererAPI
	{
		None = 0, OpenGL = 1
	};
	
	class Renderer
	{
	public:
/// @brief Get the current renderer API
/// @return The current renderer API of the game engine
		inline static RendererAPI GetAPI() { return s_RendererAPI; }
	private:
		static RendererAPI s_RendererAPI;
	};
}