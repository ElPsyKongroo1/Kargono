#include "Input.h"
#include "../../Rendering/Library.h"
#include "MiscFunctions.h"
#include "Default-Input-Libraries/Default3DFunctions.h"
#include "Default-Input-Libraries/Default2DFunctions.h"
#include "Default-Input-Libraries/DebugMenuFunctions.h"

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
    Create3DInput(default3DInput);
    Create2DInput(default2DInput);
    CreateMenuInput(debugMenuInput);
}
void InputManager::DestroyInputs()
{
    DestroyInput(default3DInput);
    DestroyInput(debugMenuInput);
    DestroyInput(default2DInput);
}
void InputManager::DestroyInput(GLInput& input)
{
    int currentArraySize;

    currentArraySize = sizeof(input.gamePadClick) / sizeof(GLClickLink);
    for (int i = 0; i < currentArraySize; i++)
    {
        input.gamePadClick[0][i] = GLClickLink();
    }
    input.gamePadClickSize[0] = 0;
    currentArraySize = sizeof(input.gamePadStick) / sizeof(GLJoyStickLink);
    for (int i = 0; i < currentArraySize; i++)
    {
        input.gamePadStick[0][i] = GLJoyStickLink();
    }
    input.gamePadStickSize[0] = 0;
    currentArraySize = sizeof(input.gamePadTrigger) / sizeof(GLTriggerLink);
    for (int i = 0; i < currentArraySize; i++)
    {
        input.gamePadTrigger[0][i] = GLTriggerLink();
    }
    input.gamePadTriggerSize[0] = 0;
    currentArraySize = sizeof(input.keyboardHold) / sizeof(GLHoldLink);
    for (int i = 0; i < currentArraySize; i++)
    {
        input.keyboardHold[0][i] = GLHoldLink();
    }
    input.keyboardHoldSize[0] = 0;
    currentArraySize = sizeof(input.keyboardClick) / sizeof(GLClickLink);
    for (int i = 0; i < currentArraySize; i++)
    {
        input.keyboardClick[0][i] = GLClickLink();
    }
    input.keyboardClickSize[0] = 0;
    currentArraySize = sizeof(input.mouseScroll) / sizeof(GLScrollLink);
    for (int i = 0; i < currentArraySize; i++)
    {
        input.mouseScroll[0][i] = GLScrollLink();
    }
    input.mouseScrollSize[0] = 0;
    currentArraySize = sizeof(input.mouseMovement) / sizeof(GLMouseMovementLink);
    for (int i = 0; i < currentArraySize; i++)
    {
        input.mouseMovement[0][i] = GLMouseMovementLink();
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
    input.gamePadClick[GLInput::SINGLEKEYPRESS][0].functionReference = Default3DFunctions::TOGGLE_FLASHLIGHT;
    input.gamePadClick[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_GAMEPAD_BUTTON_B;
    input.gamePadClick[GLInput::SINGLEKEYPRESS][1].functionReference = Default3DFunctions::RANDOM_FLASHLIGHT_COLOR;
    input.gamePadClick[GLInput::SINGLEKEYPRESS][2].glfwValue = GLFW_GAMEPAD_BUTTON_START;
    input.gamePadClick[GLInput::SINGLEKEYPRESS][2].functionReference = Default3DFunctions::TOGGLE_MENU;
    for (int i = 0; i < sizeof(input.gamePadClick[GLInput::SINGLEKEYPRESS]) / sizeof(GLInputLink); i++)
    {
        if (input.gamePadClick[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.gamePadClickSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

    input.gamePadStick[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_GAMEPAD_AXIS_LEFT_X;
    input.gamePadStick[GLInput::SINGLEKEYPRESS][0].functionReference = Default3DFunctions::MOVE_LEFT_RIGHT_STICK;
    input.gamePadStick[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_GAMEPAD_AXIS_LEFT_Y;
    input.gamePadStick[GLInput::SINGLEKEYPRESS][1].functionReference = Default3DFunctions::MOVE_UP_DOWN_STICK;
    input.gamePadStick[GLInput::SINGLEKEYPRESS][2].glfwValue = GLFW_GAMEPAD_AXIS_RIGHT_X;
    input.gamePadStick[GLInput::SINGLEKEYPRESS][2].functionReference = Default3DFunctions::CAMERA_YAW_STICK;
    input.gamePadStick[GLInput::SINGLEKEYPRESS][3].glfwValue = GLFW_GAMEPAD_AXIS_RIGHT_Y;
    input.gamePadStick[GLInput::SINGLEKEYPRESS][3].functionReference = Default3DFunctions::CAMERA_PITCH_STICK;
    for (int i = 0; i < sizeof(input.gamePadStick[GLInput::SINGLEKEYPRESS]) / sizeof(GLInputLink); i++)
    {
        if (input.gamePadStick[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.gamePadStickSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

    input.gamePadTrigger[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_GAMEPAD_AXIS_LEFT_TRIGGER;
    input.gamePadTrigger[GLInput::SINGLEKEYPRESS][0].functionReference = Default3DFunctions::CAMERA_FOV_TRIGGER;
    input.gamePadTrigger[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER;
    input.gamePadTrigger[GLInput::SINGLEKEYPRESS][1].functionReference = Default3DFunctions::CAMERA_SPEED_TRIGGER;
    for (int i = 0; i < sizeof(input.gamePadTrigger[GLInput::SINGLEKEYPRESS]) / sizeof(GLInputLink); i++)
    {
        if (input.gamePadTrigger[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.gamePadTriggerSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

    // Keyboard Initialization
    input.keyboardHold[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_KEY_W;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][0].functionReference = Default3DFunctions::MOVE_FORWARD_KEY;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_KEY_S;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][1].functionReference = Default3DFunctions::MOVE_BACKWARD_KEY;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][2].glfwValue = GLFW_KEY_D;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][2].functionReference = Default3DFunctions::MOVE_RIGHT_KEY;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][3].glfwValue = GLFW_KEY_A;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][3].functionReference = Default3DFunctions::MOVE_LEFT_KEY;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][4].glfwValue = GLFW_KEY_UP;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][4].functionReference = Default3DFunctions::CAMERA_PITCH_UP_KEY;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][5].glfwValue = GLFW_KEY_DOWN;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][5].functionReference = Default3DFunctions::CAMERA_PITCH_DOWN_KEY;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][6].glfwValue = GLFW_KEY_RIGHT;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][6].functionReference = Default3DFunctions::CAMERA_YAW_RIGHT_KEY;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][7].glfwValue = GLFW_KEY_LEFT;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][7].functionReference = Default3DFunctions::CAMERA_YAW_LEFT_KEY;
    for (int i = 0; i < sizeof(input.keyboardHold[GLInput::SINGLEKEYPRESS]) / sizeof(GLInputLink); i++)
    {
        if (input.keyboardHold[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.keyboardHoldSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

    input.keyboardClick[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_KEY_F;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][0].functionReference = Default3DFunctions::TOGGLE_FLASHLIGHT;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_KEY_F1;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][1].functionReference = Default3DFunctions::TOGGLE_MENU;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][2].glfwValue = GLFW_KEY_MINUS;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][2].functionReference = Default3DFunctions::CAMERA_DEINCREMENT_SENSITIVITY;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][3].glfwValue = GLFW_KEY_EQUAL;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][3].functionReference = Default3DFunctions::CAMERA_INCREMENT_SENSITIVITY;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][4].glfwValue = GLFW_KEY_LEFT_BRACKET;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][4].functionReference = Default3DFunctions::CAMERA_DEINCREMENT_SPEED;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][5].glfwValue = GLFW_KEY_RIGHT_BRACKET;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][5].functionReference = Default3DFunctions::CAMERA_INCREMENT_SPEED;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][6].glfwValue = GLFW_KEY_F9;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][6].functionReference = Default3DFunctions::TOGGLE_DEVICE_MOUSE_MOVEMENT;
    for (int i = 0; i < sizeof(input.keyboardClick[GLInput::SINGLEKEYPRESS]) / sizeof(GLInputLink); i++)
    {
        if (input.keyboardClick[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.keyboardClickSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

    // Mouse Scroll Initialization
    input.mouseScroll[GLInput::SINGLEKEYPRESS][0].glfwValue = 1;
    input.mouseScroll[GLInput::SINGLEKEYPRESS][0].functionReference = Default3DFunctions::CAMERA_FOV_MOUSE;
    for (int i = 0; i < sizeof(input.mouseScroll[GLInput::SINGLEKEYPRESS]) / sizeof(GLInputLink); i++)
    {
        if (input.mouseScroll[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.mouseScrollSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

    // Mouse Movement Initialization
    input.mouseMovement[GLInput::SINGLEKEYPRESS][0].glfwValue = 1;
    input.mouseMovement[GLInput::SINGLEKEYPRESS][0].functionReference = Default3DFunctions::CAMERA_YAW_PITCH_MOUSE;
    for (int i = 0; i < sizeof(input.mouseMovement[GLInput::SINGLEKEYPRESS]) / sizeof(GLInputLink); i++)
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
    input.gamePadClick[GLInput::SINGLEKEYPRESS][0].functionReference = Default2DFunctions::TOGGLE_FLASHLIGHT;
    input.gamePadClick[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_GAMEPAD_BUTTON_B;
    input.gamePadClick[GLInput::SINGLEKEYPRESS][1].functionReference = Default2DFunctions::RANDOM_FLASHLIGHT_COLOR;
    input.gamePadClick[GLInput::SINGLEKEYPRESS][2].glfwValue = GLFW_GAMEPAD_BUTTON_START;
    input.gamePadClick[GLInput::SINGLEKEYPRESS][2].functionReference = Default2DFunctions::TOGGLE_MENU;
    for (int i = 0; i < sizeof(input.gamePadClick[GLInput::SINGLEKEYPRESS]) / sizeof(GLInputLink); i++)
    {
        if (input.gamePadClick[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.gamePadClickSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

    input.gamePadStick[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_GAMEPAD_AXIS_LEFT_X;
    input.gamePadStick[GLInput::SINGLEKEYPRESS][0].functionReference = Default2DFunctions::MOVE_LEFT_RIGHT_STICK_2D;
    input.gamePadStick[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_GAMEPAD_AXIS_LEFT_Y;
    input.gamePadStick[GLInput::SINGLEKEYPRESS][1].functionReference = Default2DFunctions::MOVE_UP_DOWN_STICK_2D;
    for (int i = 0; i < sizeof(input.gamePadStick[GLInput::SINGLEKEYPRESS]) / sizeof(GLInputLink); i++)
    {
        if (input.gamePadStick[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.gamePadStickSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

    input.gamePadTrigger[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER;
    input.gamePadTrigger[GLInput::SINGLEKEYPRESS][0].functionReference = Default2DFunctions::CAMERA_SPEED_TRIGGER;
    for (int i = 0; i < sizeof(input.gamePadTrigger[GLInput::SINGLEKEYPRESS]) / sizeof(GLInputLink); i++)
    {
        if (input.gamePadTrigger[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.gamePadTriggerSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

    // Keyboard Initialization
    input.keyboardHold[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_KEY_W;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][0].functionReference = Default2DFunctions::MOVE_UP_2D;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_KEY_S;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][1].functionReference = Default2DFunctions::MOVE_DOWN_2D;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][2].glfwValue = GLFW_KEY_D;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][2].functionReference = Default2DFunctions::MOVE_RIGHT_2D;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][3].glfwValue = GLFW_KEY_A;
    input.keyboardHold[GLInput::SINGLEKEYPRESS][3].functionReference = Default2DFunctions::MOVE_LEFT_2D;
    for (int i = 0; i < sizeof(input.keyboardHold[GLInput::SINGLEKEYPRESS]) / sizeof(GLInputLink); i++)
    {
        if (input.keyboardHold[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.keyboardHoldSize[GLInput::SINGLEKEYPRESS]++;
        }
    }
    input.keyboardHold[GLInput::DOUBLEKEYPRESS][0].glfwValue = GLFW_KEY_W;
    input.keyboardHold[GLInput::DOUBLEKEYPRESS][0].glfwMask = GLFW_KEY_A;
    input.keyboardHold[GLInput::DOUBLEKEYPRESS][0].functionReference = Default2DFunctions::MOVE_UP_LEFT_2D;
    input.keyboardHold[GLInput::DOUBLEKEYPRESS][1].glfwValue = GLFW_KEY_W;
    input.keyboardHold[GLInput::DOUBLEKEYPRESS][1].glfwMask = GLFW_KEY_D;
    input.keyboardHold[GLInput::DOUBLEKEYPRESS][1].functionReference = Default2DFunctions::MOVE_UP_RIGHT_2D;
    input.keyboardHold[GLInput::DOUBLEKEYPRESS][2].glfwValue = GLFW_KEY_S;
    input.keyboardHold[GLInput::DOUBLEKEYPRESS][2].glfwMask = GLFW_KEY_A;
    input.keyboardHold[GLInput::DOUBLEKEYPRESS][2].functionReference = Default2DFunctions::MOVE_DOWN_LEFT_2D;
    input.keyboardHold[GLInput::DOUBLEKEYPRESS][3].glfwValue = GLFW_KEY_S;
    input.keyboardHold[GLInput::DOUBLEKEYPRESS][3].glfwMask = GLFW_KEY_D;
    input.keyboardHold[GLInput::DOUBLEKEYPRESS][3].functionReference = Default2DFunctions::MOVE_DOWN_RIGHT_2D;
    for (int i = 0; i < sizeof(input.keyboardHold[1]) / sizeof(GLInputLink); i++)
    {
        if (input.keyboardHold[GLInput::DOUBLEKEYPRESS][i].glfwValue != -1)
        {
            input.keyboardHoldSize[GLInput::DOUBLEKEYPRESS]++;
        }
    }

    input.keyboardClick[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_KEY_F;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][0].functionReference = Default2DFunctions::TOGGLE_FLASHLIGHT;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_KEY_F1;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][1].functionReference = Default2DFunctions::TOGGLE_MENU;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][2].glfwValue = GLFW_KEY_MINUS;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][2].functionReference = Default2DFunctions::CAMERA_DEINCREMENT_SENSITIVITY;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][3].glfwValue = GLFW_KEY_EQUAL;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][3].functionReference = Default2DFunctions::CAMERA_INCREMENT_SENSITIVITY;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][4].glfwValue = GLFW_KEY_LEFT_BRACKET;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][4].functionReference = Default2DFunctions::CAMERA_DEINCREMENT_SPEED;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][5].glfwValue = GLFW_KEY_RIGHT_BRACKET;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][5].functionReference = Default2DFunctions::CAMERA_INCREMENT_SPEED;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][6].glfwValue = GLFW_KEY_F9;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][6].functionReference = Default2DFunctions::TOGGLE_DEVICE_MOUSE_MOVEMENT;
    for (int i = 0; i < sizeof(input.keyboardClick[GLInput::SINGLEKEYPRESS]) / sizeof(GLInputLink); i++)
    {
        if (input.keyboardClick[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.keyboardClickSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

    // Mouse Scroll Initialization
    input.mouseScroll[GLInput::SINGLEKEYPRESS][0].glfwValue = 1;
    input.mouseScroll[GLInput::SINGLEKEYPRESS][0].functionReference = Default2DFunctions::CAMERA_FOV_MOUSE;
    for (int i = 0; i < sizeof(input.mouseScroll[GLInput::SINGLEKEYPRESS]) / sizeof(GLInputLink); i++)
    {
        if (input.mouseScroll[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.mouseScrollSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

    // Mouse Movement Initialization
    for (int i = 0; i < sizeof(input.mouseMovement[GLInput::SINGLEKEYPRESS]) / sizeof(GLInputLink); i++)
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
    input.gamePadClick[GLInput::SINGLEKEYPRESS][0].functionReference = DebugMenuFunctions::TOGGLE_MENU;
    input.gamePadClick[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_GAMEPAD_BUTTON_B;
    input.gamePadClick[GLInput::SINGLEKEYPRESS][1].functionReference = DebugMenuFunctions::CLOSE_CURRENT_WINDOW;
    for (int i = 0; i < sizeof(input.gamePadClick[GLInput::SINGLEKEYPRESS]) / sizeof(GLInputLink); i++)
    {
        if (input.gamePadClick[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.gamePadClickSize[GLInput::SINGLEKEYPRESS]++;
        }
    }
    for (int i = 0; i < sizeof(input.gamePadStick[GLInput::SINGLEKEYPRESS]) / sizeof(GLInputLink); i++)
    {
        if (input.gamePadStick[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.gamePadStickSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

    for (int i = 0; i < sizeof(input.gamePadTrigger[GLInput::SINGLEKEYPRESS]) / sizeof(GLInputLink); i++)
    {
        if (input.gamePadTrigger[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.gamePadTriggerSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

    // Keyboard Initialization
    for (int i = 0; i < sizeof(input.keyboardHold[GLInput::SINGLEKEYPRESS]) / sizeof(GLInputLink); i++)
    {
        if (input.keyboardHold[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.keyboardHoldSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

    input.keyboardClick[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_KEY_F1;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][0].functionReference = DebugMenuFunctions::TOGGLE_MENU;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_KEY_TAB;
    input.keyboardClick[GLInput::SINGLEKEYPRESS][1].functionReference = DebugMenuFunctions::CLOSE_CURRENT_WINDOW;
    for (int i = 0; i < sizeof(input.keyboardClick[GLInput::SINGLEKEYPRESS]) / sizeof(GLInputLink); i++)
    {
        if (input.keyboardClick[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.keyboardClickSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

    // Mouse Scroll Initialization
    for (int i = 0; i < sizeof(input.mouseScroll[GLInput::SINGLEKEYPRESS]) / sizeof(GLInputLink); i++)
    {
        if (input.mouseScroll[GLInput::SINGLEKEYPRESS][i].glfwValue != -1)
        {
            input.mouseScrollSize[GLInput::SINGLEKEYPRESS]++;
        }
    }

    // Mouse Movement Initialization
    for (int i = 0; i < sizeof(input.mouseMovement[GLInput::SINGLEKEYPRESS]) / sizeof(GLInputLink); i++)
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




