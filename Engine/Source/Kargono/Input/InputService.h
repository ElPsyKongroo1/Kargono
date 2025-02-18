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
		// Poll Input from Platform Dependent Backend
		//==============================
		static bool IsKeyPressed(KeyCode key);
		static bool IsMouseButtonPressed(MouseCode button);
		static Math::vec2 GetAbsoluteMousePosition();
		static float GetAbsoluteMouseX();
		static float GetAbsoluteMouseY();

		//==============================
		// Poll Input from Application
		//==============================
		static Math::vec2 GetViewportMousePosition();
	};

}
