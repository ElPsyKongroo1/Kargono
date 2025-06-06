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
		GLFWwindow* window = (GLFWwindow*)EngineService::GetActiveWindow().GetNativeWindow();
		int state = glfwGetKey(window, static_cast<int32_t>(key));
		return state == GLFW_PRESS;

	}
	bool InputService::IsMouseButtonPressed(const MouseCode button)
	{
		GLFWwindow* window = (GLFWwindow*)EngineService::GetActiveWindow().GetNativeWindow();
		int state = glfwGetMouseButton(window, static_cast<int32_t>(button));
		return state == GLFW_PRESS;
	}
	Math::vec2 InputService::GetAbsoluteMousePosition()
	{
		GLFWwindow* window = (GLFWwindow*)EngineService::GetActiveWindow().GetNativeWindow();
		double xpos;
		double ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return { (float)xpos, (float)ypos };
	}
	float InputService::GetAbsoluteMouseX()
	{
		return GetAbsoluteMousePosition().x;
	}

	float InputService::GetAbsoluteMouseY()
	{
		return GetAbsoluteMousePosition().y;
	}

	Math::vec2 InputService::GetViewportMousePosition()
	{
		Application* currentApp = EngineService::GetActiveApplication();
		KG_ASSERT(currentApp);

		return currentApp->GetMouseViewportPosition();
	}

}

#endif
