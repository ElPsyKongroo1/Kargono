/// @brief Include the precompiled header file for Kargono game engine
#include "Kargono/kgpch.h"
/// @brief Include the WindowsInput header file
#include "WindowsInput.h"

#include "Kargono/Application.h"
/// @brief Include the GLFW header file
#include <GLFW/glfw3.h>

/// @brief Namespace for the Kargono game engine
namespace Kargono
{
/// @brief Initialize the s_Instance pointer with a new instance of WindowsInput
	Input* Input::s_Instance = new WindowsInput();

/// @brief Implementation of the IsKeyPressed function for WindowsInput
	bool Kargono::WindowsInput::IsKeyPressedImpl(int keycode)
	{
/// @brief Cast the native window from the Application class as GLFWwindow
/// @brief Cast the native window from the Application class as GLFWwindow
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
/// @brief Get the state of the specified key using the GLFW library
		auto state = glfwGetKey(window, keycode);
/// @brief Return true if the key is currently pressed or being held down
		return state == GLFW_PRESS || state == GLFW_REPEAT;

	}
/// @brief Implementation of the IsMouseButtonPressed function for WindowsInput
	bool WindowsInput::IsMouseButtonPressedImpl(int button)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
/// @brief Get the state of the specified mouse button using the GLFW library
		auto state = glfwGetMouseButton(window, button);
/// @brief Return true if the mouse button is currently pressed
		return state == GLFW_PRESS;
	}
/// @brief Implementation of the GetMousePosition function for WindowsInput
	std::pair<float, float> WindowsInput::GetMousePositionImpl()
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		double xpos;
		double ypos;
/// @brief Get the current position of the cursor using the GLFW library
		glfwGetCursorPos(window, &xpos, &ypos);

/// @brief Return the cursor position as a pair of floats
		return { (float)xpos, (float)ypos };
	}
/// @brief Implementation of the GetMouseX function for WindowsInput
	bool WindowsInput::GetMouseXImpl()
	{
/// @brief Get the current cursor position using the GetMousePositionImpl function
/// @brief Get the current cursor position using the GetMousePositionImpl function
		auto [x, y] = GetMousePositionImpl();
/// @brief Return the x-coordinate of the cursor position
		return x;
	}

/// @brief Implementation of the GetMouseY function for WindowsInput
	bool WindowsInput::GetMouseYImpl()
	{
		auto [x, y] = GetMousePositionImpl();
/// @brief Return the y-coordinate of the cursor position
		return y;
	}

}
