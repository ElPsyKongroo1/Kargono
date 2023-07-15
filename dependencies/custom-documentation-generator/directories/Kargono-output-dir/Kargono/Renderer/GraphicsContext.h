#pragma once

/// @brief Namespace for Kargono game engine
namespace Kargono
{
/// @brief Class representing a graphics context in the game engine
	class GraphicsContext
	{
	public:
/// @brief Initializes the graphics context
		virtual void Init() = 0;
/// @brief Swaps the front and back buffers to display the rendered frame
		virtual void SwapBuffers() = 0;
	};

}