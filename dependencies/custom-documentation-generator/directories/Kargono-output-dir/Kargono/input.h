#pragma once
#include "Kargono/Core.h"

namespace Kargono
{
/// @class Input
	class KG_API Input
	{
	public:
/// @param keycode The key code of the key being checked
/// @return True if the key is currently pressed, false otherwise
		inline static bool IsKeyPressed(int keycode) { return s_Instance->IsKeyPressedImpl(keycode); }
/// @param button The mouse button being checked
/// @return True if the mouse button is currently pressed, false otherwise
		inline static bool IsMouseButtonPressed(int button) { return s_Instance->IsMouseButtonPressedImpl(button); }
/// @return A pair of floats representing the x and y coordinates of the mouse position
		inline static std::pair<float, float> GetMousePosition() { return s_Instance->GetMousePositionImpl(); }
/// @return The x coordinate of the mouse position
		inline static bool GetMouseX() { return s_Instance->GetMouseXImpl(); }
/// @return The y coordinate of the mouse position
		inline static bool GetMouseY() { return s_Instance->GetMouseYImpl(); }

/// @details The protected section of the Input class
	protected:
/// @param keycode The key code of the key being checked
/// @return True if the key is currently pressed, false otherwise
		virtual bool IsKeyPressedImpl(int keycode) = 0;
/// @param button The mouse button being checked
/// @return True if the mouse button is currently pressed, false otherwise
		virtual bool IsMouseButtonPressedImpl(int button) = 0;
/// @return A pair of floats representing the x and y coordinates of the mouse position
		virtual std::pair<float, float> GetMousePositionImpl() = 0;
/// @return The x coordinate of the mouse position
		virtual bool GetMouseXImpl() = 0;
/// @return The y coordinate of the mouse position
		virtual bool GetMouseYImpl() = 0;
/// @details The private section of the Input class
	private:
/// @details The static instance of the Input class
		static Input* s_Instance;

	};

}