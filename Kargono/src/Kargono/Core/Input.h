#pragma once
#include "Kargono/Core/KeyCodes.h"
#include "Kargono/Core/MouseCodes.h"

#include <glm/glm.hpp>

namespace Kargono
{
	class Input
	{
	public:
		static bool IsKeyPressed(KeyCode key);
		static bool IsMouseButtonPressed(MouseCode button);
		static glm::vec2 GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();

	};

}
