#include "kgpch.h"

#include "Kargono/Input/InputService.h"
#include "Kargono/Core/Engine.h"
#include "Kargono/Math/Math.h"

#if defined(KG_PLATFORM_WINDOWS) || defined(KG_PLATFORM_LINUX) 

#include "API/Platform/GlfwAPI.h"

namespace Kargono::Input
{
	bool InputService::IsKeyPressed(const KeyCode key)
	{
		if (key == 0) 
		{
			return false;
		}
		auto* window = static_cast<GLFWwindow*>(EngineService::GetActiveWindow().GetNativeWindow());
		auto state = glfwGetKey(window, static_cast<int32_t>(key));
		return state == GLFW_PRESS;

	}
	bool InputService::IsMouseButtonPressed(const MouseCode button)
	{
		auto* window = static_cast<GLFWwindow*>(EngineService::GetActiveWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));
		return state == GLFW_PRESS;
	}
	Math::vec2 InputService::GetMousePosition()
	{
		auto* window = static_cast<GLFWwindow*>(EngineService::GetActiveWindow().GetNativeWindow());
		double xpos;
		double ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return { (float)xpos, (float)ypos };
	}
	float InputService::GetMouseX()
	{
		return GetMousePosition().x;
	}

	float InputService::GetMouseY()
	{
		return GetMousePosition().y;
	}

}

#endif
