#include "Input.h"
#include "../Library.h"
#include "InputFunctions.h"

/*============================================================================================================================================================================================
 *============================================================================================================================================================================================
 * GLInput Class
 *============================================================================================================================================================================================
 *============================================================================================================================================================================================*/

 /*============================================================================================================================================================================================
  * External Functionality
  *============================================================================================================================================================================================*/


void GLInput::processScroll(double xoffset, double yoffset)
{
    if (!isMouseScroll) { return; }
    updateType = false;

    for (int i = 0; i < mouseScrollSize[0]; i++)
    {
        Resources::inputManager.accessScrollFunctions(mouseScroll[0][i].function, xoffset, yoffset);
    }
    if (updateType)
    {
        updateInputType();
    }
}

void GLInput::processMouseMovement(double xpos, double ypos)
{
    if (!isMouseMovement) { return; }
    updateType = false;

    for (int i = 0; i < mouseMovementSize[0]; i++)
    {
        Resources::inputManager.accessMouseMovementFunctions(mouseMovement[0][i].function, xpos, ypos);
    }
    if (updateType)
    {
        updateInputType();
    }
}

void GLInput::processKeyboardClick(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (!isKeyboardClick) { return; }
    updateType = false;

    for (int i = 0; i < keyboardClickSize[0]; i++)
    {
        if (key == keyboardClick[0][i].glfwValue && action == GLFW_PRESS)
        {
            Resources::inputManager.accessClickFunction(keyboardClick[0][i].function, nullptr);
        }
    }
    if (updateType)
    {
        updateInputType();
    }
}

void GLInput::processKeyboardHold(GLFWwindow* window)
{
    if (!isKeyboardHold) { return; }

    updateType = false;
    int shouldCalcDir = 0;
    bool shouldSkipButton = false;
    std::vector<int> disabledButtons = std::vector<int>();

    for (int i = 0; i < keyboardHoldSize[1]; i++)
    {
        shouldSkipButton = false;
        for (int j = 0; j < disabledButtons.size(); j++)
        {
            if (keyboardHold[1][i].glfwValue == disabledButtons.at(j))
            {
                shouldSkipButton = true;
                break;
            }
        }
        if (shouldSkipButton) continue;


        if (glfwGetKey(window, keyboardHold[1][i].glfwValue) == GLFW_PRESS &&
            glfwGetKey(window, keyboardHold[1][i].glfwMask) == GLFW_PRESS)
        {
            shouldCalcDir += Resources::inputManager.accessHoldFunctions(keyboardHold[1][i].function);
            disabledButtons.push_back(keyboardHold[1][i].glfwValue);
            disabledButtons.push_back(keyboardHold[1][i].glfwMask);
        }
    }

    
    for (int i = 0; i < keyboardHoldSize[0]; i++)
    {
        shouldSkipButton = false;
        for (int j = 0; j < disabledButtons.size(); j++)
        {
            if (keyboardHold[0][i].glfwValue == disabledButtons.at(j))
            {
                shouldSkipButton = true;
                break;
            }
        }
        if (shouldSkipButton) continue;

        if (glfwGetKey(window, keyboardHold[0][i].glfwValue) == GLFW_PRESS)
        {
            shouldCalcDir += Resources::inputManager.accessHoldFunctions(keyboardHold[0][i].function);
        }
    }
    if (shouldCalcDir > 0)
    {
        glm::vec3 direction;
        direction.x = cos(glm::radians(Resources::currentApplication->currentCamera->eulerAngle.yaw)) * cos(glm::radians(Resources::currentApplication->currentCamera->eulerAngle.pitch));
        direction.y = sin(glm::radians(Resources::currentApplication->currentCamera->eulerAngle.pitch));
        direction.z = sin(glm::radians(Resources::currentApplication->currentCamera->eulerAngle.yaw)) * cos(glm::radians(Resources::currentApplication->currentCamera->eulerAngle.pitch));
        Resources::currentApplication->currentCamera->orientation.cameraFront = glm::normalize(direction);
    }
    if (updateType)
    {
        updateInputType();
    }
}



void GLInput::processGamePad()
{
    int axesCount;
    const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axesCount);
    
    
    processGamePadStick(axes);

    processGamePadTrigger(axes);

    processGamePadClick();

}

void GLInput::processGamePadStick(const float* axes)
{
    if (!isGamePadStick) { return; }
    updateType = false;

    int shouldCalcDir = 0;
    for (int i = 0; i < gamePadStickSize[0]; i++)
    {
        if (fabs(axes[gamePadStick[0][i].glfwValue]) > 0.1f)
        {
            shouldCalcDir += Resources::inputManager.accessJoyStickFunctions(gamePadStick[0][i].function, axes[gamePadStick[0][i].glfwValue]);
        }
    }
    if (shouldCalcDir > 0)
    {
        glm::vec3 direction;
        direction.x = cos(glm::radians(Resources::currentApplication->currentCamera->eulerAngle.yaw)) * cos(glm::radians(Resources::currentApplication->currentCamera->eulerAngle.pitch));
        direction.y = sin(glm::radians(Resources::currentApplication->currentCamera->eulerAngle.pitch));
        direction.z = sin(glm::radians(Resources::currentApplication->currentCamera->eulerAngle.yaw)) * cos(glm::radians(Resources::currentApplication->currentCamera->eulerAngle.pitch));
        Resources::currentApplication->currentCamera->orientation.cameraFront = glm::normalize(direction);
    }
    if (updateType)
    {
        updateInputType();
    }
}

void GLInput::processGamePadTrigger(const float* axes)
{
    if (!isGamePadTrigger) { return; }
    updateType = false;

    for (int i = 0; i < gamePadTriggerSize[0]; i++)
    {
        if (axes[gamePadTrigger[0][i].glfwValue] > -0.9f)
        {
            Resources::inputManager.accessTriggerFunctions(gamePadTrigger[0][i].function, axes[gamePadTrigger[0][i].glfwValue]);
        }
    }
    if (updateType)
    {
        updateInputType();
    }
}

void GLInput::processGamePadClick()
{
    if (!isGamePadClick) { return; }
    updateType = false;

    int buttonCount;
    const unsigned char* currentButtons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonCount);

    for (int i = 0; i < gamePadClickSize[0]; i++)
    {
        
        if (gamePadClick[0][i].previousState == currentButtons[gamePadClick[0][i].glfwValue])
        {
            // Do nothing
        }
        else
        {
            std::cout << gamePadClickSize[0] << std::endl;
            if (currentButtons[gamePadClick[0][i].glfwValue] == GLFW_PRESS)
            {
                Resources::inputManager.accessClickFunction(gamePadClick[0][i].function, &gamePadClick[0][i]);

            }
            if (currentButtons[gamePadClick[0][i].glfwValue] == GLFW_RELEASE)
            {
                // Do something on release!
            }
            gamePadClick[0][i].previousState = currentButtons[gamePadClick[0][i].glfwValue];
        }
    }
    if (updateType)
    {
        updateInputType();
    }
}



