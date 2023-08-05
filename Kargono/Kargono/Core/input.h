#pragma once
#include "Kargono/Core/Base.h"
#include "Kargono/Core/KeyCodes.h"
#include "Kargono/Core/MouseCodes.h"

namespace Kargono
{
	class Input
	{
	public:
		static bool IsKeyPressed(KeyCode key);
		static bool IsMouseButtonPressed(MouseCode button);
		static std::pair<float, float> GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();

	};

}