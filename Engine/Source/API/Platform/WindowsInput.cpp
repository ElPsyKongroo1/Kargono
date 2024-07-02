#include "kgpch.h"

#include "Kargono/Input/InputPolling.h"
#include "Kargono/Core/Engine.h"
#include "Kargono/Math/Math.h"

#include "API/Platform/GlfwAPI.h"

#ifdef KG_PLATFORM_WINDOWS

namespace Kargono::Input
{
	bool InputPolling::IsKeyPressed(const KeyCode key)
	{
		auto* window = static_cast<GLFWwindow*>(EngineService::GetActiveWindow().GetNativeWindow());
		auto state = glfwGetKey(window, static_cast<int32_t>(key));
		return state == GLFW_PRESS;

	}
	bool InputPolling::IsMouseButtonPressed(const MouseCode button)
	{
		auto* window = static_cast<GLFWwindow*>(EngineService::GetActiveWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));
		return state == GLFW_PRESS;
	}
	Math::vec2 InputPolling::GetMousePosition()
	{
		auto* window = static_cast<GLFWwindow*>(EngineService::GetActiveWindow().GetNativeWindow());
		double xpos;
		double ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return { (float)xpos, (float)ypos };
	}
	float InputPolling::GetMouseX()
	{
		return GetMousePosition().x;
	}

	float InputPolling::GetMouseY()
	{
		return GetMousePosition().y;
	}

}

#endif
