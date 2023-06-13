#include "Input.h"
#include "../../Rendering/Library.h"
#include "MiscFunctions.h"
#include "Default-Input-Libraries/Default3DFunctions.h"
#include "Default-Input-Libraries/Default2DFunctions.h"
#include "Default-Input-Libraries/DebugMenuFunctions.h"
#include <algorithm>

/*============================================================================================================================================================================================
 *============================================================================================================================================================================================
 * GLInput Class
 *============================================================================================================================================================================================
 *============================================================================================================================================================================================*/

GLInput::GLInput(bool valPadClick, bool valKeyHold, bool valMouseScroll,
    bool valPadStick, bool valKeyClick, bool valMouseMove,
    bool valPadTrigger, GLClickLink gamePadClick[2][128],
    GLJoyStickLink gamePadStick[2][32], GLTriggerLink gamePadTrigger[2][8],
    GLHoldLink keyboardHold[2][128], GLClickLink keyboardClick[2][128],
    GLScrollLink mouseScroll[2][2], GLMouseMovementLink mouseMovement[2][2])
{

    // Toggle Device Input
    isGamePadClick = valPadClick; isGamePadStick = valPadStick, isGamePadTrigger = valPadTrigger;
    isKeyboardHold = valKeyHold, isKeyboardClick = valKeyClick;
    isMouseScroll = valMouseScroll, isMouseMovement = valMouseMove;

    // GamePad Click Initialization
    for (int i = 0; i < sizeof(gamePadClick[GLInput::SINGLEKEYPRESS]) / sizeof(GLClickLink); i++)
    {
        if (gamePadClick[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            this->gamePadClickSize[GLInput::SINGLEKEYPRESS]++;
        }
    }
    if (gamePadClickSize[GLInput::SINGLEKEYPRESS] > 0) 
    {
        std::copy(gamePadClick[0], (gamePadClick[0] + gamePadClickSize[GLInput::SINGLEKEYPRESS]), this->gamePadClick[0]);
    }
    // GamePad Stick Initialization
    for (int i = 0; i < sizeof(gamePadStick[GLInput::SINGLEKEYPRESS]) / sizeof(GLJoyStickLink); i++)
    {
        if (gamePadStick[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            this->gamePadStickSize[GLInput::SINGLEKEYPRESS]++;
        }
    }
    if (gamePadStickSize[GLInput::SINGLEKEYPRESS] > 0)
    {
        std::copy(gamePadStick[0], (gamePadStick[0] + gamePadStickSize[GLInput::SINGLEKEYPRESS]), this->gamePadStick[0]);
    }
    // GamePad Trigger Initialization
    for (int i = 0; i < sizeof(gamePadTrigger[GLInput::SINGLEKEYPRESS]) / sizeof(GLTriggerLink); i++)
    {
        if (gamePadTrigger[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            this->gamePadTriggerSize[GLInput::SINGLEKEYPRESS]++;
        }
    }
    if (gamePadTriggerSize[GLInput::SINGLEKEYPRESS] > 0)
    {
        std::copy(gamePadTrigger[0], (gamePadTrigger[0] + gamePadTriggerSize[GLInput::SINGLEKEYPRESS]), this->gamePadTrigger[0]);
    }
    // Keyboard Hold Initialization
    for (int i = 0; i < sizeof(keyboardHold[GLInput::SINGLEKEYPRESS]) / sizeof(GLHoldLink); i++)
    {
        if (keyboardHold[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            this->keyboardHoldSize[GLInput::SINGLEKEYPRESS]++;
        }
    }
    if (keyboardHoldSize[GLInput::SINGLEKEYPRESS] > 0)
    {
        std::copy(keyboardHold[0], (keyboardHold[0] + keyboardHoldSize[GLInput::SINGLEKEYPRESS]), this->keyboardHold[0]);
    }
    for (int i = 0; i < sizeof(keyboardHold[1]) / sizeof(GLHoldLink); i++)
    {
        if (keyboardHold[GLInput::DOUBLEKEYPRESS][i].glfwValue != -1)
        {
            this->keyboardHoldSize[GLInput::DOUBLEKEYPRESS]++;
        }
    }
    if (keyboardHoldSize[GLInput::DOUBLEKEYPRESS] > 0)
    {
        std::copy(keyboardHold[1], (keyboardHold[1] + keyboardHoldSize[GLInput::DOUBLEKEYPRESS]), this->keyboardHold[1]);
    }
    // Keyboard Click Initialization
    for (int i = 0; i < sizeof(keyboardClick[GLInput::SINGLEKEYPRESS]) / sizeof(GLClickLink); i++)
    {
        if (keyboardClick[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            this->keyboardClickSize[GLInput::SINGLEKEYPRESS]++;
        }
    }
    if (keyboardClickSize[GLInput::SINGLEKEYPRESS] > 0)
    {
        std::copy(keyboardClick[0], (keyboardClick[0] + keyboardClickSize[GLInput::SINGLEKEYPRESS]), this->keyboardClick[0]);
    }
    // Mouse Scroll Initialization
    for (int i = 0; i < sizeof(mouseScroll[GLInput::SINGLEKEYPRESS]) / sizeof(GLScrollLink); i++)
    {
        if (mouseScroll[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            this->mouseScrollSize[GLInput::SINGLEKEYPRESS]++;
        }
    }
    if (mouseScrollSize[GLInput::SINGLEKEYPRESS] > 0)
    {
        std::copy(mouseScroll[0], (mouseScroll[0] + mouseScrollSize[GLInput::SINGLEKEYPRESS]), this->mouseScroll[0]);
    }
    // Mouse Movement Initialization
    for (int i = 0; i < sizeof(mouseMovement[GLInput::SINGLEKEYPRESS]) / sizeof(GLMouseMovementLink); i++)
    {
        if (mouseMovement[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            this->mouseMovementSize[GLInput::SINGLEKEYPRESS]++;
        }
    }
    if (mouseMovementSize[GLInput::SINGLEKEYPRESS] > 0)
    {
        std::copy(mouseMovement[0], (mouseMovement[0] + mouseMovementSize[GLInput::SINGLEKEYPRESS]), this->mouseMovement[0]);
    }

}

GLInput::~GLInput() 
{
    int currentArraySize;

    currentArraySize = sizeof(gamePadClick) / sizeof(GLClickLink);
    for (int i = 0; i < currentArraySize; i++)
    {
        gamePadClick[0][i] = GLClickLink();
    }
    gamePadClickSize[0] = 0;
    currentArraySize = sizeof(gamePadStick) / sizeof(GLJoyStickLink);
    for (int i = 0; i < currentArraySize; i++)
    {
        gamePadStick[0][i] = GLJoyStickLink();
    }
    gamePadStickSize[0] = 0;
    currentArraySize = sizeof(gamePadTrigger) / sizeof(GLTriggerLink);
    for (int i = 0; i < currentArraySize; i++)
    {
        gamePadTrigger[0][i] = GLTriggerLink();
    }
    gamePadTriggerSize[0] = 0;
    currentArraySize = sizeof(keyboardHold) / sizeof(GLHoldLink);
    for (int i = 0; i < currentArraySize; i++)
    {
        keyboardHold[0][i] = GLHoldLink();
    }
    keyboardHoldSize[0] = 0;
    currentArraySize = sizeof(keyboardClick) / sizeof(GLClickLink);
    for (int i = 0; i < currentArraySize; i++)
    {
        keyboardClick[0][i] = GLClickLink();
    }
    keyboardClickSize[0] = 0;
    currentArraySize = sizeof(mouseScroll) / sizeof(GLScrollLink);
    for (int i = 0; i < currentArraySize; i++)
    {
        mouseScroll[0][i] = GLScrollLink();
    }
    mouseScrollSize[0] = 0;
    currentArraySize = sizeof(mouseMovement) / sizeof(GLMouseMovementLink);
    for (int i = 0; i < currentArraySize; i++)
    {
        mouseMovement[0][i] = GLMouseMovementLink();
    }
    mouseMovementSize[0] = 0;

    isGamePadClick = false; isGamePadStick = false; isGamePadTrigger = false;
    isKeyboardHold = false; isKeyboardClick = false;
    isMouseScroll = false; isMouseMovement = false;
}

 /*============================================================================================================================================================================================
  * External Functionality
  *============================================================================================================================================================================================*/


void GLInput::processScroll(double xoffset, double yoffset)
{
    if (!isMouseScroll) { return; }
    updateType = false;

    for (int i = 0; i < mouseScrollSize[0]; i++)
    {
       mouseScroll[0][i].functionReference(xoffset, yoffset);
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
        mouseMovement[0][i].functionReference(xpos, ypos) ;
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
            keyboardClick[0][i].functionReference(nullptr);
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
            shouldCalcDir += keyboardHold[1][i].functionReference();
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
            shouldCalcDir += keyboardHold[0][i].functionReference();
        }
    }
    if (shouldCalcDir > 0)
    {
        glm::vec3 direction;
        direction.x = cos(glm::radians(Resources::currentRenderer->currentCamera->eulerAngle.yaw)) * cos(glm::radians(Resources::currentRenderer->currentCamera->eulerAngle.pitch));
        direction.y = sin(glm::radians(Resources::currentRenderer->currentCamera->eulerAngle.pitch));
        direction.z = sin(glm::radians(Resources::currentRenderer->currentCamera->eulerAngle.yaw)) * cos(glm::radians(Resources::currentRenderer->currentCamera->eulerAngle.pitch));
        Resources::currentRenderer->currentCamera->orientation.cameraFront = glm::normalize(direction);
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
            shouldCalcDir += gamePadStick[0][i].functionReference(axes[gamePadStick[0][i].glfwValue]);
        }
    }
    if (shouldCalcDir > 0)
    {
        glm::vec3 direction;
        direction.x = cos(glm::radians(Resources::currentRenderer->currentCamera->eulerAngle.yaw)) * cos(glm::radians(Resources::currentRenderer->currentCamera->eulerAngle.pitch));
        direction.y = sin(glm::radians(Resources::currentRenderer->currentCamera->eulerAngle.pitch));
        direction.z = sin(glm::radians(Resources::currentRenderer->currentCamera->eulerAngle.yaw)) * cos(glm::radians(Resources::currentRenderer->currentCamera->eulerAngle.pitch));
        Resources::currentRenderer->currentCamera->orientation.cameraFront = glm::normalize(direction);
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
            gamePadTrigger[0][i].functionReference(axes[gamePadTrigger[0][i].glfwValue]);
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
                gamePadClick[0][i].functionReference(&gamePadClick[0][i]);

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



