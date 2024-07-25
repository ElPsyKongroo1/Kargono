#pragma once
#include "Kargono/Core/KeyCodes.h"
#include "Kargono/Core/MouseCodes.h"
#include "Kargono/Math/Math.h"


namespace Kargono::Input
{
	//==============================
	// Input Polling Class
	//==============================
	class InputService
	{
	public:
		//==============================
		// Poll Input from Platform Dependent Code
		//==============================
		static bool IsKeyPressed(KeyCode key);
		static bool IsMouseButtonPressed(MouseCode button);
		static Math::vec2 GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
	};

}
