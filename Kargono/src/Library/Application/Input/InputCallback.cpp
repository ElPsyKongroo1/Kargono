#include "Kargono/kgpch.h"
#include "InputCallback.h"

/*============================================================================================================================================================================================
 * Input/Callback
 *============================================================================================================================================================================================*/
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    Resources::currentApplication->currentInput->processMouseMovement(xpos, ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    Resources::currentApplication->currentInput->processScroll(xoffset, yoffset);
}

void FProcessInputHold(GLFWwindow* window)
{
    Resources::currentApplication->currentInput->processKeyboardHold(window);
    int present = glfwJoystickPresent(GLFW_JOYSTICK_1);
    if (!present)
    {
        // Nothing, the controller is not connected
    }
    else
    {
        Resources::currentApplication->currentInput->processGamePad();
    }

}

void UKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Resources::currentApplication->currentInput->processKeyboardClick(window, key, scancode, action, mods);
}