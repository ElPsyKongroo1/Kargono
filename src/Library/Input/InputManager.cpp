#include "Input.h"
#include "../Library.h"
#include "InputFunctions.h"

/*============================================================================================================================================================================================
 *============================================================================================================================================================================================
 * InputManager Class
 *============================================================================================================================================================================================
 *============================================================================================================================================================================================*/

 /*============================================================================================================================================================================================
  * Instantiate/Delete GLInput objects
  *============================================================================================================================================================================================*/

void InputManager::CreateInputs()
{
    Create3DInput(input3D);
    Create2DInput(input2D);
    CreateMenuInput(menuInput);
}
void InputManager::DestroyInputs()
{
    DestroyInput(input3D);
    DestroyInput(menuInput);
    DestroyInput(input2D);
}
void InputManager::DestroyInput(GLInput& input)
{
    int currentArraySize;

    currentArraySize = sizeof(input.gamePadClick) / sizeof(GLButton);
    for (int i = 0; i < currentArraySize; i++)
    {
        input.gamePadClick[0][i] = GLButton();
    }
    input.gamePadClickSize[0] = 0;
    currentArraySize = sizeof(input.gamePadStick) / sizeof(GLButton);
    for (int i = 0; i < currentArraySize; i++)
    {
        input.gamePadStick[0][i] = GLButton();
    }
    input.gamePadStickSize[0] = 0;
    currentArraySize = sizeof(input.gamePadTrigger) / sizeof(GLButton);
    for (int i = 0; i < currentArraySize; i++)
    {
        input.gamePadTrigger[0][i] = GLButton();
    }
    input.gamePadTriggerSize[0] = 0;
    currentArraySize = sizeof(input.keyboardHold) / sizeof(GLButton);
    for (int i = 0; i < currentArraySize; i++)
    {
        input.keyboardHold[0][i] = GLButton();
    }
    input.keyboardHoldSize[0] = 0;
    currentArraySize = sizeof(input.keyboardClick) / sizeof(GLButton);
    for (int i = 0; i < currentArraySize; i++)
    {
        input.keyboardClick[0][i] = GLButton();
    }
    input.keyboardClickSize[0] = 0;
    currentArraySize = sizeof(input.mouseScroll) / sizeof(GLButton);
    for (int i = 0; i < currentArraySize; i++)
    {
        input.mouseScroll[0][i] = GLButton();
    }
    input.mouseScrollSize[0] = 0;
    currentArraySize = sizeof(input.mouseMovement) / sizeof(GLButton);
    for (int i = 0; i < currentArraySize; i++)
    {
        input.mouseMovement[0][i] = GLButton();
    }
    input.mouseMovementSize[0] = 0;

    input.isGamePadClick = false; input.isGamePadStick = false; input.isGamePadTrigger = false;
    input.isKeyboardHold = false; input.isKeyboardClick = false;
    input.isMouseScroll = false; input.isMouseMovement = false;
}
/*============================================================================================================================================================================================
 * Initializes GLInput Structs with specified values
 *============================================================================================================================================================================================*/
void InputManager::Create3DInput(GLInput& input)
{
    // Toggle Device Input
    input.isGamePadClick = true; input.isGamePadStick = true, input.isGamePadTrigger = true;
    input.isKeyboardHold = true, input.isKeyboardClick = true;
    input.isMouseScroll = true, input.isMouseMovement = true;

    // GamePad Initialization
    input.gamePadClick[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_GAMEPAD_BUTTON_Y;
    input.gamePadClick[GLInput::SINGLEKEYPRESS][0].function = FunctionReferences::TOGGLE_FLASHLIGHT;
    input.gamePadClick[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_GAMEPAD_BUTTON_B;
    input.gamePadClick[GLInput::SINGLEKEYPRESS][1].function = FunctionReferences::RANDOM_FLASHLIGHT_COLOR;
    input.gamePadClick[GLInput::SINGLEKEYPRESS][2].glfwValue = GLFW_GAMEPAD_BUTTON_START;
    input.gamePadClick[GLInput::SINGLEKEYPRESS][2].function = FunctionReferences::TOGGLE_MENU;
    for (int i = 0; i < sizeof(input.gamePadClick[GLInput::SINGLEKEYPRESS]) / sizeof(GLButton); i++)
    {
        if (input.gamePadClick[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.gamePadClickSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

    input.gamePadStick[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_GAMEPAD_AXIS_LEFT_X;
    input.gamePadStick[GLInput::SINGLEKEYPRESS][0].function = FunctionReferences::MOVE_LEFT_RIGHT_STICK;
    input.gamePadStick[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_GAMEPAD_AXIS_LEFT_Y;
    input.gamePadStick[GLInput::SINGLEKEYPRESS][1].function = FunctionReferences::MOVE_UP_DOWN_STICK;
    input.gamePadStick[GLInput::SINGLEKEYPRESS][2].glfwValue = GLFW_GAMEPAD_AXIS_RIGHT_X;
    input.gamePadStick[GLInput::SINGLEKEYPRESS][2].function = FunctionReferences::CAMERA_YAW_STICK;
    input.gamePadStick[GLInput::SINGLEKEYPRESS][3].glfwValue = GLFW_GAMEPAD_AXIS_RIGHT_Y;
    input.gamePadStick[GLInput::SINGLEKEYPRESS][3].function = FunctionReferences::CAMERA_PITCH_STICK;
    for (int i = 0; i < sizeof(input.gamePadStick[GLInput::SINGLEKEYPRESS]) / sizeof(GLButton); i++)
    {
        if (input.gamePadStick[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.gamePadStickSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

    input.gamePadTrigger[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_GAMEPAD_AXIS_LEFT_TRIGGER;
    input.gamePadTrigger[GLInput::SINGLEKEYPRESS][0].function = FunctionReferences::CAMERA_FOV_TRIGGER;
    input.gamePadTrigger[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER;
    input.gamePadTrigger[GLInput::SINGLEKEYPRESS][1].function = FunctionReferences::CAMERA_SPEED_TRIGGER;
    for (int i = 0; i < sizeof(input.gamePadTrigger[GLInput::SINGLEKEYPRESS]) / sizeof(GLButton); i++)
    {
        if (input.gamePadTrigger[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.gamePadTriggerSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

    // Keyboard Initialization
    input.keyboardHold[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_KEY_W;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][0].function = FunctionReferences::MOVE_FORWARD_KEY;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_KEY_S;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][1].function = FunctionReferences::MOVE_BACKWARD_KEY;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][2].glfwValue = GLFW_KEY_D;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][2].function = FunctionReferences::MOVE_RIGHT_KEY;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][3].glfwValue = GLFW_KEY_A;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][3].function = FunctionReferences::MOVE_LEFT_KEY;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][4].glfwValue = GLFW_KEY_UP;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][4].function = FunctionReferences::CAMERA_PITCH_UP_KEY;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][5].glfwValue = GLFW_KEY_DOWN;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][5].function = FunctionReferences::CAMERA_PITCH_DOWN_KEY;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][6].glfwValue = GLFW_KEY_RIGHT;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][6].function = FunctionReferences::CAMERA_YAW_RIGHT_KEY;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][7].glfwValue = GLFW_KEY_LEFT;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][7].function = FunctionReferences::CAMERA_YAW_LEFT_KEY;
    for (int i = 0; i < sizeof(input.keyboardHold[GLInput::SINGLEKEYPRESS]) / sizeof(GLButton); i++)
    {
        if (input.keyboardHold[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.keyboardHoldSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

    input.keyboardClick[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_KEY_F;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][0].function = FunctionReferences::TOGGLE_FLASHLIGHT;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_KEY_F1;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][1].function = FunctionReferences::TOGGLE_MENU;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][2].glfwValue = GLFW_KEY_MINUS;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][2].function = FunctionReferences::CAMERA_DEINCREMENT_SENSITIVITY;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][3].glfwValue = GLFW_KEY_EQUAL;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][3].function = FunctionReferences::CAMERA_INCREMENT_SENSITIVITY;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][4].glfwValue = GLFW_KEY_LEFT_BRACKET;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][4].function = FunctionReferences::CAMERA_DEINCREMENT_SPEED;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][5].glfwValue = GLFW_KEY_RIGHT_BRACKET;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][5].function = FunctionReferences::CAMERA_INCREMENT_SPEED;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][6].glfwValue = GLFW_KEY_F9;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][6].function = FunctionReferences::TOGGLE_DEVICE_MOUSE_MOVEMENT;
    for (int i = 0; i < sizeof(input.keyboardClick[GLInput::SINGLEKEYPRESS]) / sizeof(GLButton); i++)
    {
        if (input.keyboardClick[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.keyboardClickSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

    // Mouse Scroll Initialization
    input.mouseScroll[GLInput::SINGLEKEYPRESS][0].glfwValue = 1;
    input.mouseScroll[GLInput::SINGLEKEYPRESS][0].function = FunctionReferences::CAMERA_FOV_MOUSE;
    for (int i = 0; i < sizeof(input.mouseScroll[GLInput::SINGLEKEYPRESS]) / sizeof(GLButton); i++)
    {
        if (input.mouseScroll[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.mouseScrollSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

    // Mouse Movement Initialization
    input.mouseMovement[GLInput::SINGLEKEYPRESS][0].glfwValue = 1;
    input.mouseMovement[GLInput::SINGLEKEYPRESS][0].function = FunctionReferences::CAMERA_YAW_PITCH_MOUSE;
    for (int i = 0; i < sizeof(input.mouseMovement[GLInput::SINGLEKEYPRESS]) / sizeof(GLButton); i++)
    {
        if (input.mouseMovement[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.mouseMovementSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

}

void InputManager::Create2DInput(GLInput& input)
{
    // Toggle Device Input
    input.isGamePadClick = true; input.isGamePadStick = true, input.isGamePadTrigger = true;
    input.isKeyboardHold = true, input.isKeyboardClick = true;
    input.isMouseScroll = true, input.isMouseMovement = true;

    // GamePad Initialization
    input.gamePadClick[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_GAMEPAD_BUTTON_Y;
    input.gamePadClick[GLInput::SINGLEKEYPRESS][0].function = FunctionReferences::TOGGLE_FLASHLIGHT;
    input.gamePadClick[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_GAMEPAD_BUTTON_B;
    input.gamePadClick[GLInput::SINGLEKEYPRESS][1].function = FunctionReferences::RANDOM_FLASHLIGHT_COLOR;
    input.gamePadClick[GLInput::SINGLEKEYPRESS][2].glfwValue = GLFW_GAMEPAD_BUTTON_START;
    input.gamePadClick[GLInput::SINGLEKEYPRESS][2].function = FunctionReferences::TOGGLE_MENU;
    for (int i = 0; i < sizeof(input.gamePadClick[GLInput::SINGLEKEYPRESS]) / sizeof(GLButton); i++)
    {
        if (input.gamePadClick[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.gamePadClickSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

    input.gamePadStick[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_GAMEPAD_AXIS_LEFT_X;
    input.gamePadStick[GLInput::SINGLEKEYPRESS][0].function = FunctionReferences::MOVE_LEFT_RIGHT_STICK_2D;
    input.gamePadStick[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_GAMEPAD_AXIS_LEFT_Y;
    input.gamePadStick[GLInput::SINGLEKEYPRESS][1].function = FunctionReferences::MOVE_UP_DOWN_STICK_2D;
    for (int i = 0; i < sizeof(input.gamePadStick[GLInput::SINGLEKEYPRESS]) / sizeof(GLButton); i++)
    {
        if (input.gamePadStick[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.gamePadStickSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

    input.gamePadTrigger[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER;
    input.gamePadTrigger[GLInput::SINGLEKEYPRESS][0].function = FunctionReferences::CAMERA_SPEED_TRIGGER;
    for (int i = 0; i < sizeof(input.gamePadTrigger[GLInput::SINGLEKEYPRESS]) / sizeof(GLButton); i++)
    {
        if (input.gamePadTrigger[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.gamePadTriggerSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

    // Keyboard Initialization
    input.keyboardHold[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_KEY_W;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][0].function = FunctionReferences::MOVE_UP_2D;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_KEY_S;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][1].function = FunctionReferences::MOVE_DOWN_2D;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][2].glfwValue = GLFW_KEY_D;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][2].function = FunctionReferences::MOVE_RIGHT_2D;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][3].glfwValue = GLFW_KEY_A;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][3].function = FunctionReferences::MOVE_LEFT_2D;
    for (int i = 0; i < sizeof(input.keyboardHold[GLInput::SINGLEKEYPRESS]) / sizeof(GLButton); i++)
    {
        if (input.keyboardHold[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.keyboardHoldSize[GLInput::SINGLEKEYPRESS]++;
        }
    }
    input.keyboardHold[GLInput::DOUBLEKEYPRESS][0].glfwValue = GLFW_KEY_W;
    input.keyboardHold[GLInput::DOUBLEKEYPRESS][0].glfwMask = GLFW_KEY_A;
    input.keyboardHold[GLInput::DOUBLEKEYPRESS][0].function = FunctionReferences::MOVE_UP_LEFT_2D;
    input.keyboardHold[GLInput::DOUBLEKEYPRESS][1].glfwValue = GLFW_KEY_W;
    input.keyboardHold[GLInput::DOUBLEKEYPRESS][1].glfwMask = GLFW_KEY_D;
    input.keyboardHold[GLInput::DOUBLEKEYPRESS][1].function = FunctionReferences::MOVE_UP_RIGHT_2D;
    input.keyboardHold[GLInput::DOUBLEKEYPRESS][2].glfwValue = GLFW_KEY_S;
    input.keyboardHold[GLInput::DOUBLEKEYPRESS][2].glfwMask = GLFW_KEY_A;
    input.keyboardHold[GLInput::DOUBLEKEYPRESS][2].function = FunctionReferences::MOVE_DOWN_LEFT_2D;
    input.keyboardHold[GLInput::DOUBLEKEYPRESS][3].glfwValue = GLFW_KEY_S;
    input.keyboardHold[GLInput::DOUBLEKEYPRESS][3].glfwMask = GLFW_KEY_D;
    input.keyboardHold[GLInput::DOUBLEKEYPRESS][3].function = FunctionReferences::MOVE_DOWN_RIGHT_2D;
    for (int i = 0; i < sizeof(input.keyboardHold[1]) / sizeof(GLButton); i++)
    {
        if (input.keyboardHold[GLInput::DOUBLEKEYPRESS][i].glfwValue != -1)
        {
            input.keyboardHoldSize[GLInput::DOUBLEKEYPRESS]++;
        }
    }

    input.keyboardClick[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_KEY_F;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][0].function = FunctionReferences::TOGGLE_FLASHLIGHT;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_KEY_F1;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][1].function = FunctionReferences::TOGGLE_MENU;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][2].glfwValue = GLFW_KEY_MINUS;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][2].function = FunctionReferences::CAMERA_DEINCREMENT_SENSITIVITY;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][3].glfwValue = GLFW_KEY_EQUAL;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][3].function = FunctionReferences::CAMERA_INCREMENT_SENSITIVITY;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][4].glfwValue = GLFW_KEY_LEFT_BRACKET;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][4].function = FunctionReferences::CAMERA_DEINCREMENT_SPEED;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][5].glfwValue = GLFW_KEY_RIGHT_BRACKET;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][5].function = FunctionReferences::CAMERA_INCREMENT_SPEED;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][6].glfwValue = GLFW_KEY_F9;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][6].function = FunctionReferences::TOGGLE_DEVICE_MOUSE_MOVEMENT;
    for (int i = 0; i < sizeof(input.keyboardClick[GLInput::SINGLEKEYPRESS]) / sizeof(GLButton); i++)
    {
        if (input.keyboardClick[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.keyboardClickSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

    // Mouse Scroll Initialization
    input.mouseScroll[GLInput::SINGLEKEYPRESS][0].glfwValue = 1;
    input.mouseScroll[GLInput::SINGLEKEYPRESS][0].function = FunctionReferences::CAMERA_FOV_MOUSE;
    for (int i = 0; i < sizeof(input.mouseScroll[GLInput::SINGLEKEYPRESS]) / sizeof(GLButton); i++)
    {
        if (input.mouseScroll[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.mouseScrollSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

    // Mouse Movement Initialization
    for (int i = 0; i < sizeof(input.mouseMovement[GLInput::SINGLEKEYPRESS]) / sizeof(GLButton); i++)
    {
        if (input.mouseMovement[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.mouseMovementSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

}

void InputManager::CreateMenuInput(GLInput& input)
{
    // Toggle Device Input
    input.isGamePadClick = true; input.isGamePadStick = true, input.isGamePadTrigger = true;
    input.isKeyboardHold = true, input.isKeyboardClick = true;
    input.isMouseScroll = true, input.isMouseMovement = true;

    // GamePad Initialization
    input.gamePadClick[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_GAMEPAD_BUTTON_START;
    input.gamePadClick[GLInput::SINGLEKEYPRESS][0].function = FunctionReferences::TOGGLE_MENU;
    input.gamePadClick[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_GAMEPAD_BUTTON_B;
    input.gamePadClick[GLInput::SINGLEKEYPRESS][1].function = FunctionReferences::CLOSE_CURRENT_WINDOW;
    for (int i = 0; i < sizeof(input.gamePadClick[GLInput::SINGLEKEYPRESS]) / sizeof(GLButton); i++)
    {
        if (input.gamePadClick[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.gamePadClickSize[GLInput::SINGLEKEYPRESS]++;
        }
    }
    for (int i = 0; i < sizeof(input.gamePadStick[GLInput::SINGLEKEYPRESS]) / sizeof(GLButton); i++)
    {
        if (input.gamePadStick[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.gamePadStickSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

    for (int i = 0; i < sizeof(input.gamePadTrigger[GLInput::SINGLEKEYPRESS]) / sizeof(GLButton); i++)
    {
        if (input.gamePadTrigger[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.gamePadTriggerSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

    // Keyboard Initialization
    for (int i = 0; i < sizeof(input.keyboardHold[GLInput::SINGLEKEYPRESS]) / sizeof(GLButton); i++)
    {
        if (input.keyboardHold[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.keyboardHoldSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

    input.keyboardClick[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_KEY_F1;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][0].function = FunctionReferences::TOGGLE_MENU;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_KEY_TAB;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][1].function = FunctionReferences::CLOSE_CURRENT_WINDOW;
    for (int i = 0; i < sizeof(input.keyboardClick[GLInput::SINGLEKEYPRESS]) / sizeof(GLButton); i++)
    {
        if (input.keyboardClick[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.keyboardClickSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

    // Mouse Scroll Initialization
    for (int i = 0; i < sizeof(input.mouseScroll[GLInput::SINGLEKEYPRESS]) / sizeof(GLButton); i++)
    {
        if (input.mouseScroll[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.mouseScrollSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

    // Mouse Movement Initialization
    for (int i = 0; i < sizeof(input.mouseMovement[GLInput::SINGLEKEYPRESS]) / sizeof(GLButton); i++)
    {
        if (input.mouseMovement[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.mouseMovementSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

}

 /*============================================================================================================================================================================================
 * External Functionality
 *============================================================================================================================================================================================*/


bool InputManager::accessHoldFunctions(FunctionReferences index)
{
    float cameraSpeed;

    switch (index)
    {
    case MOVE_FORWARD_KEY:
        cameraSpeed = Resources::currentApplication->currentCamera->currentCameraSpeed * Resources::deltaTime;
        Resources::currentApplication->currentCamera->cameraPosition += cameraSpeed * Resources::currentApplication->currentCamera->cameraFront;
        break;
    case MOVE_BACKWARD_KEY:
        cameraSpeed = Resources::currentApplication->currentCamera->currentCameraSpeed * Resources::deltaTime;
        Resources::currentApplication->currentCamera->cameraPosition -= cameraSpeed * Resources::currentApplication->currentCamera->cameraFront;
        break;
    case MOVE_LEFT_KEY:
        cameraSpeed = Resources::currentApplication->currentCamera->currentCameraSpeed * Resources::deltaTime;
        Resources::currentApplication->currentCamera->cameraPosition -= glm::normalize(glm::cross(Resources::currentApplication->currentCamera->cameraFront, Resources::currentApplication->currentCamera->cameraUp)) * cameraSpeed;
        break;
    case MOVE_RIGHT_KEY:
        cameraSpeed = Resources::currentApplication->currentCamera->currentCameraSpeed * Resources::deltaTime;
        Resources::currentApplication->currentCamera->cameraPosition += glm::normalize(glm::cross(Resources::currentApplication->currentCamera->cameraFront, Resources::currentApplication->currentCamera->cameraUp)) * cameraSpeed;
        break;
    case CAMERA_PITCH_UP_KEY:
        Resources::currentApplication->currentCamera->pitch += Resources::currentApplication->currentCamera->currentSensitivity;
        if (Resources::currentApplication->currentCamera->pitch > 89.0f)
            Resources::currentApplication->currentCamera->pitch = 89.0f;
        if (Resources::currentApplication->currentCamera->pitch < -89.0f)
            Resources::currentApplication->currentCamera->pitch = -89.0f;
        return true;
        break;
    case CAMERA_PITCH_DOWN_KEY:
        Resources::currentApplication->currentCamera->pitch -= Resources::currentApplication->currentCamera->currentSensitivity;
        if (Resources::currentApplication->currentCamera->pitch > 89.0f)
            Resources::currentApplication->currentCamera->pitch = 89.0f;
        if (Resources::currentApplication->currentCamera->pitch < -89.0f)
            Resources::currentApplication->currentCamera->pitch = -89.0f;
        return true;
        break;
    case CAMERA_YAW_LEFT_KEY:
        Resources::currentApplication->currentCamera->yaw -= Resources::currentApplication->currentCamera->currentSensitivity;
        return true;
        break;
    case CAMERA_YAW_RIGHT_KEY:
        Resources::currentApplication->currentCamera->yaw += Resources::currentApplication->currentCamera->currentSensitivity;
        return true;
        break;
    case MOVE_UP_2D:
        cameraSpeed = Resources::currentApplication->currentCamera->currentCameraSpeed * Resources::deltaTime;
        Resources::currentApplication->currentCamera->cameraPosition += glm::vec3(0.0f, cameraSpeed, 0.0f);
        break;
    case MOVE_DOWN_2D:
        cameraSpeed = Resources::currentApplication->currentCamera->currentCameraSpeed * Resources::deltaTime;
        Resources::currentApplication->currentCamera->cameraPosition -= glm::vec3(0.0f, cameraSpeed, 0.0f);
        break;
    case MOVE_RIGHT_2D:
        cameraSpeed = Resources::currentApplication->currentCamera->currentCameraSpeed * Resources::deltaTime;
        Resources::currentApplication->currentCamera->cameraPosition += glm::vec3(cameraSpeed, 0.0f, 0.0f);
        break;
    case MOVE_LEFT_2D:
        cameraSpeed = Resources::currentApplication->currentCamera->currentCameraSpeed * Resources::deltaTime;
        Resources::currentApplication->currentCamera->cameraPosition -= glm::vec3(cameraSpeed, 0.0f, 0.0f);
        break;
    case MOVE_UP_LEFT_2D:
        cameraSpeed = Resources::currentApplication->currentCamera->currentCameraDiagonal * Resources::deltaTime;
        Resources::currentApplication->currentCamera->cameraPosition += glm::vec3(0.0f, cameraSpeed, 0.0f);
        Resources::currentApplication->currentCamera->cameraPosition -= glm::vec3(cameraSpeed, 0.0f, 0.0f);
        break;
    case MOVE_DOWN_LEFT_2D:
        cameraSpeed = Resources::currentApplication->currentCamera->currentCameraDiagonal * Resources::deltaTime;
        Resources::currentApplication->currentCamera->cameraPosition -= glm::vec3(0.0f, cameraSpeed, 0.0f);
        Resources::currentApplication->currentCamera->cameraPosition -= glm::vec3(cameraSpeed, 0.0f, 0.0f);
        break;
    case MOVE_UP_RIGHT_2D:
        cameraSpeed = Resources::currentApplication->currentCamera->currentCameraDiagonal * Resources::deltaTime;
        Resources::currentApplication->currentCamera->cameraPosition += glm::vec3(0.0f, cameraSpeed, 0.0f);
        Resources::currentApplication->currentCamera->cameraPosition += glm::vec3(cameraSpeed, 0.0f, 0.0f);
        break;
    case MOVE_DOWN_RIGHT_2D:
        cameraSpeed = Resources::currentApplication->currentCamera->currentCameraDiagonal * Resources::deltaTime;
        Resources::currentApplication->currentCamera->cameraPosition -= glm::vec3(0.0f, cameraSpeed, 0.0f);
        Resources::currentApplication->currentCamera->cameraPosition += glm::vec3(cameraSpeed, 0.0f, 0.0f);
        break;
    default:
        std::cerr << "ERROR::INVALID_ENUM::ACCESS_HOLD_FUNCTION" << std::endl;
        throw std::runtime_error("Check Logs");
        break;
    }
    return false;
}


bool InputManager::accessClickFunction(FunctionReferences index, GLButton* gamePadButton)
{
    bool currentFlash;
    bool decision;
    bool foundFlashLight;
    glm::vec3 randomColor;
    float r, g, b;

    switch (index)
    {
    case TOGGLE_FLASHLIGHT:
        currentFlash = Resources::currentApplication->currentCamera->isFlashLight;
        if (currentFlash)
        {
            decision = false;
        }
        else
        {
            decision = true;
        }
        foundFlashLight = false;
        if (decision)
        {
            Resources::currentApplication->currentCamera->isFlashLight = true;
            for (int i = 0; i < Resources::currentApplication->lightSourceRenderBuffer.size(); i++)
            {
                if (&Resources::currentApplication->currentCamera->flashLight == Resources::currentApplication->lightSourceRenderBuffer.at(i))
                {
                    foundFlashLight = true;
                    break;
                }
            }
            if (!foundFlashLight)
            {
                Resources::currentApplication->lightSourceRenderBuffer.push_back(&Resources::currentApplication->currentCamera->flashLight);
            }
        }
        else
        {
            Resources::currentApplication->currentCamera->isFlashLight = false;
            for (int i = 0; i < Resources::currentApplication->lightSourceRenderBuffer.size(); i++)
            {
                if (&Resources::currentApplication->currentCamera->flashLight == Resources::currentApplication->lightSourceRenderBuffer.at(i))
                {
                    foundFlashLight = true;
                    Resources::currentApplication->lightSourceRenderBuffer.erase(Resources::currentApplication->lightSourceRenderBuffer.begin() + i);
                    break;
                }
            }   
        }
        break;
    case TOGGLE_DEVICE_MOUSE_MOVEMENT:
        if (Resources::currentApplication->currentInput->isMouseMovement) { Resources::currentApplication->currentInput->isMouseMovement = false; }
        else { Resources::currentApplication->currentInput->isMouseMovement = true; Resources::currentApplication->currentCamera->firstMouse = true; }
        break;
    case EXIT_APPLICATION:
        //glfwSetWindowMonitor(Resources::currentApplication->window, NULL, 0, 0, Resources::currentApplication->screenDimension.x, Resources::currentApplication->screenDimension.y, NULL);
        glfwSetWindowShouldClose(Resources::currentApplication->window, true);
        break;
    case RANDOM_FLASHLIGHT_COLOR:
        r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        if (r < 0.4f)
        {
            r += 0.4f;
        }
        if (r < 0.7f)
        {
            r += 0.2f;
        }
        g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        if (g < 0.4f)
        {
            g += 0.4f;
        }
        if (g < 0.7f)
        {
            g += 0.2f;
        }
        b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        if (b < 0.4f)
        {
            b += 0.4f;
        }
        if (b < 0.7f)
        {
            b += 0.2f;
        }
        randomColor = glm::vec3(r, g, b);
        Resources::currentApplication->currentCamera->flashLight.ambientColor = randomColor * 0.1f;
        Resources::currentApplication->currentCamera->flashLight.diffuseColor = randomColor;
        Resources::currentApplication->currentCamera->flashLight.specularColor = randomColor;
        break;
    case TOGGLE_MENU:
        if (Resources::currentApplication->currentInput != &Resources::inputManager.menuInput)
        {
            glfwSetInputMode(Resources::currentApplication->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            updateType = true;
            typeChange[0] = Resources::currentApplication->currentInput;
            typeChange[1] = &Resources::inputManager.menuInput;
            if (gamePadButton != nullptr) oldButton = gamePadButton;
            Resources::currentApplication->currentCamera->firstMouse = true;
            Resources::windowManager.mainMenu.isRendering = true;
            Resources::currentApplication->currentWindow = &Resources::windowManager.mainMenu;
        }
        else if (Resources::currentApplication->currentInput == &Resources::inputManager.menuInput)
        {
            glfwSetInputMode(Resources::currentApplication->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            updateType = true;
            typeChange[0] = Resources::currentApplication->currentInput;
            typeChange[1] = Resources::currentApplication->defaultInput;
            Resources::windowManager.mainMenu.closeChildren();
            Resources::currentApplication->currentWindow = nullptr;
        }
        return true;
        break;
    case CLOSE_CURRENT_WINDOW:
        Resources::currentApplication->currentWindow->isRendering = false;
        Resources::currentApplication->currentWindow = Resources::currentApplication->currentWindow->parent;
        if (Resources::currentApplication->currentWindow == nullptr)
        {
            glfwSetInputMode(Resources::currentApplication->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            updateType = true;
            typeChange[0] = Resources::currentApplication->currentInput;
            typeChange[1] = Resources::currentApplication->defaultInput;

            Resources::windowManager.mainMenu.closeChildren();
            Resources::currentApplication->currentWindow = nullptr;
        }
        break;
    case CAMERA_DEINCREMENT_SENSITIVITY:
        if ((Resources::currentApplication->currentCamera->currentSensitivity - Resources::currentApplication->currentCamera->slowSensitivity) < Resources::currentApplication->currentCamera->slowSensitivity)
            return false;
        Resources::currentApplication->currentCamera->currentSensitivity -= Resources::currentApplication->currentCamera->slowSensitivity;
        break;
    case CAMERA_INCREMENT_SENSITIVITY:
        if ((Resources::currentApplication->currentCamera->currentSensitivity + Resources::currentApplication->currentCamera->slowSensitivity) > (Resources::currentApplication->currentCamera->slowSensitivity * 10))
            return false;
        Resources::currentApplication->currentCamera->currentSensitivity += Resources::currentApplication->currentCamera->slowSensitivity;
        break;
    case CAMERA_DEINCREMENT_SPEED:
        if ((Resources::currentApplication->currentCamera->currentCameraSpeed - Resources::currentApplication->currentCamera->slowCameraSpeed) < Resources::currentApplication->currentCamera->slowCameraSpeed)
            return false;
        Resources::currentApplication->currentCamera->setCurrentCameraSpeed(Resources::currentApplication->currentCamera->currentCameraSpeed - Resources::currentApplication->currentCamera->slowCameraSpeed);
        break;
    case CAMERA_INCREMENT_SPEED:
        if ((Resources::currentApplication->currentCamera->currentCameraSpeed + Resources::currentApplication->currentCamera->slowCameraSpeed) > (Resources::currentApplication->currentCamera->slowCameraSpeed * 50))
            return false;
        Resources::currentApplication->currentCamera->setCurrentCameraSpeed(Resources::currentApplication->currentCamera->currentCameraSpeed + Resources::currentApplication->currentCamera->slowCameraSpeed);
        break;
    default:
        std::cerr << "ERROR::INVALID_ENUM::ACCESS_CLICK_FUNCTION" << std::endl;
        throw std::runtime_error("Check Logs");
        break;
    }
    return false;
}

void InputManager::accessScrollFunctions(FunctionReferences index, double xoffset, double yoffset)
{
    switch (index)
    {
    case CAMERA_FOV_MOUSE:
        Resources::currentApplication->currentCamera->fov -= (float)yoffset;
        if (Resources::currentApplication->currentCamera->fov < 1.0f)
            Resources::currentApplication->currentCamera->fov = 1.0f;
        if (Resources::currentApplication->currentCamera->fov > 45.0f)
            Resources::currentApplication->currentCamera->fov = 45.0f;
        break;
    default:
        std::cerr << "ERROR::INVALID_ENUM::ACCESS_SCROLL_FUNCTION" << std::endl;
        throw std::runtime_error("Check Logs");
        break;
    }
}
void InputManager::accessMouseMovementFunctions(FunctionReferences index, double xpos, double ypos)
{
    float xoffset;
    float yoffset;
    glm::vec3 direction;

    switch (index)
    {
    case CAMERA_YAW_PITCH_MOUSE:
        if (Resources::currentApplication->currentCamera->firstMouse) // initially set to true
        {
            Resources::currentApplication->currentCamera->lastX = xpos;
            Resources::currentApplication->currentCamera->lastY = ypos;
            Resources::currentApplication->currentCamera->firstMouse = false;
        }

        xoffset = xpos - Resources::currentApplication->currentCamera->lastX;
        yoffset = Resources::currentApplication->currentCamera->lastY - ypos; // reversed since y-coordinates range from bottom to top
        Resources::currentApplication->currentCamera->lastX = xpos;
        Resources::currentApplication->currentCamera->lastY = ypos;

        xoffset *= Resources::currentApplication->currentCamera->currentSensitivity;
        yoffset *= Resources::currentApplication->currentCamera->currentSensitivity;

        Resources::currentApplication->currentCamera->yaw += xoffset;
        Resources::currentApplication->currentCamera->pitch += yoffset;

        if (Resources::currentApplication->currentCamera->pitch > 89.0f)
            Resources::currentApplication->currentCamera->pitch = 89.0f;
        if (Resources::currentApplication->currentCamera->pitch < -89.0f)
            Resources::currentApplication->currentCamera->pitch = -89.0f;


        direction.x = cos(glm::radians(Resources::currentApplication->currentCamera->yaw)) * cos(glm::radians(Resources::currentApplication->currentCamera->pitch));
        direction.y = sin(glm::radians(Resources::currentApplication->currentCamera->pitch));
        direction.z = sin(glm::radians(Resources::currentApplication->currentCamera->yaw)) * cos(glm::radians(Resources::currentApplication->currentCamera->pitch));
        Resources::currentApplication->currentCamera->cameraFront = glm::normalize(direction);
        break;
    default:
        std::cerr << "ERROR::INVALID_ENUM::ACCESS_MOUSE_MOVEMENT_FUNCTION" << std::endl;
        throw std::runtime_error("Check Logs");
        break;
    }
}

bool InputManager::accessJoyStickFunctions(FunctionReferences index, float axis)
{
    float cameraSpeed = Resources::currentApplication->currentCamera->currentCameraSpeed * Resources::deltaTime;

    switch (index)
    {
    case MOVE_LEFT_RIGHT_STICK:
        cameraSpeed = Resources::currentApplication->currentCamera->currentCameraSpeed * Resources::deltaTime;
        Resources::currentApplication->currentCamera->cameraPosition += glm::normalize(glm::cross(Resources::currentApplication->currentCamera->cameraFront, Resources::currentApplication->currentCamera->cameraUp)) * (cameraSpeed * axis);
        break;
    case MOVE_UP_DOWN_STICK:
        cameraSpeed = Resources::currentApplication->currentCamera->currentCameraSpeed * Resources::deltaTime;
        Resources::currentApplication->currentCamera->cameraPosition -= (cameraSpeed * axis) * Resources::currentApplication->currentCamera->cameraFront;
        break;
    case MOVE_LEFT_RIGHT_STICK_2D:
        cameraSpeed = Resources::currentApplication->currentCamera->currentCameraSpeed * Resources::deltaTime;
        Resources::currentApplication->currentCamera->cameraPosition += glm::vec3(cameraSpeed * axis, 0.0f, 0.0f);
        break;
    case MOVE_UP_DOWN_STICK_2D:
        cameraSpeed = Resources::currentApplication->currentCamera->currentCameraSpeed * Resources::deltaTime;
        Resources::currentApplication->currentCamera->cameraPosition -= glm::vec3(0.0f, cameraSpeed * axis, 0.0f);
        break;
    case CAMERA_YAW_STICK:
        Resources::currentApplication->currentCamera->yaw += axis;
        return true;
        break;
    case CAMERA_PITCH_STICK:
        Resources::currentApplication->currentCamera->pitch -= axis;
        if (Resources::currentApplication->currentCamera->pitch > 89.0f)
            Resources::currentApplication->currentCamera->pitch = 89.0f;
        if (Resources::currentApplication->currentCamera->pitch < -89.0f)
            Resources::currentApplication->currentCamera->pitch = -89.0f;
        return true;
        break;
    default:
        std::cerr << "ERROR::INVALID_ENUM::ACCESS_JOY_STICK_FUNCTION" << std::endl;
        throw std::runtime_error("Check Logs");
        break;
    }
    return false;
}

void InputManager::accessTriggerFunctions(FunctionReferences index, float axis)
{
    switch (index)
    {
    case CAMERA_FOV_TRIGGER:
        if (axis < -0.15)
        {
            Resources::currentApplication->currentCamera->fov = 45.0f;
        }
        if (axis < -0.5f)
        {
            Resources::currentApplication->currentCamera->fov = 34.0f;
        }
        else if (axis < 0.0f)
        {
            Resources::currentApplication->currentCamera->fov = 23.0f;
        }
        else if (axis < 0.5f)
        {
            Resources::currentApplication->currentCamera->fov = 12.0f;
        }
        else if (axis < 0.75f)
        {
            Resources::currentApplication->currentCamera->fov = 6.5f;
        }
        else if (axis < 0.875f)
        {
            Resources::currentApplication->currentCamera->fov = 3.75f;
        }
        else
        {
            Resources::currentApplication->currentCamera->fov = 1.0f;
        }
        break;
    case CAMERA_SPEED_TRIGGER:
        if (axis < -0.15)
        {
            Resources::currentApplication->currentCamera->setCurrentCameraSpeed(Resources::currentApplication->currentCamera->slowCameraSpeed);
        }
        if (axis < -0.5f)
        {
            Resources::currentApplication->currentCamera->setCurrentCameraSpeed(Resources::currentApplication->currentCamera->slowCameraSpeed * 3.0f);
        }
        else if (axis < 0.0f)
        {
            Resources::currentApplication->currentCamera->setCurrentCameraSpeed(Resources::currentApplication->currentCamera->slowCameraSpeed * 5.0f);
        }
        else if (axis < 0.5f)
        {
            Resources::currentApplication->currentCamera->setCurrentCameraSpeed(Resources::currentApplication->currentCamera->slowCameraSpeed * 7.0f);
        }
        else if (axis < 0.75f)
        {
            Resources::currentApplication->currentCamera->setCurrentCameraSpeed(Resources::currentApplication->currentCamera->slowCameraSpeed * 8.0f);
        }
        else if (axis < 0.875f)
        {
            Resources::currentApplication->currentCamera->setCurrentCameraSpeed(Resources::currentApplication->currentCamera->slowCameraSpeed * 9.0f);
        }
        else
        {
            Resources::currentApplication->currentCamera->setCurrentCameraSpeed(Resources::currentApplication->currentCamera->slowCameraSpeed * 10.0f);
        }
        break;
    default:
        std::cerr << "ERROR::INVALID_ENUM::ACCESS_TRIGGER_FUNCTION" << std::endl;
        throw std::runtime_error("Check Logs");
        break;
    }
}

