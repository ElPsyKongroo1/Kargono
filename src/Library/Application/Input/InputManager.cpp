#include "Input.h"
#include "../../Library.h"
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
    Create3DInput();
    Create2DInput();
    CreateMenuInput();
}
void InputManager::DestroyInputs()
{
    delete default3DInput;
    delete debugMenuInput;
    delete default2DInput;
}
/*============================================================================================================================================================================================
 * Initializes GLInput Structs with specified values
 *============================================================================================================================================================================================*/
void InputManager::Create3DInput()
{
    // Toggle Device Input
    bool isGamePadClick = true; 
    bool isGamePadStick = true;
    bool isGamePadTrigger = true;
    bool isKeyboardHold = true;
    bool isKeyboardClick = true;
    bool isMouseScroll = true;
    bool isMouseMovement = true;

    auto gamePadClick{ new GLClickLink[2][128] };
    auto gamePadStick{ new GLJoyStickLink [2][32] };
    auto gamePadTrigger {new GLTriggerLink[2][8]};
    auto keyboardHold{ new GLHoldLink[2][128] };
    auto keyboardClick{ new GLClickLink[2][128] };
    auto mouseScroll{ new GLScrollLink[2][2] };
    auto mouseMovement{ new GLMouseMovementLink[2][2] };
    auto keyboardRelease{ new GLClickLink[2][128] };

    

    // GamePad Initialization
    gamePadClick[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_GAMEPAD_BUTTON_Y;
    gamePadClick[GLInput::SINGLEKEYPRESS][0].functionReference = Default3DFunctions::TOGGLE_FLASHLIGHT;
    gamePadClick[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_GAMEPAD_BUTTON_B;
    gamePadClick[GLInput::SINGLEKEYPRESS][1].functionReference = Default3DFunctions::RANDOM_FLASHLIGHT_COLOR;
    gamePadClick[GLInput::SINGLEKEYPRESS][2].glfwValue = GLFW_GAMEPAD_BUTTON_START;
    gamePadClick[GLInput::SINGLEKEYPRESS][2].functionReference = Default3DFunctions::TOGGLE_MENU;

    gamePadStick[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_GAMEPAD_AXIS_LEFT_X;
    gamePadStick[GLInput::SINGLEKEYPRESS][0].functionReference = Default3DFunctions::MOVE_LEFT_RIGHT_STICK;
    gamePadStick[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_GAMEPAD_AXIS_LEFT_Y;
    gamePadStick[GLInput::SINGLEKEYPRESS][1].functionReference = Default3DFunctions::MOVE_UP_DOWN_STICK;
    gamePadStick[GLInput::SINGLEKEYPRESS][2].glfwValue = GLFW_GAMEPAD_AXIS_RIGHT_X;
    gamePadStick[GLInput::SINGLEKEYPRESS][2].functionReference = Default3DFunctions::CAMERA_YAW_STICK;
    gamePadStick[GLInput::SINGLEKEYPRESS][3].glfwValue = GLFW_GAMEPAD_AXIS_RIGHT_Y;
    gamePadStick[GLInput::SINGLEKEYPRESS][3].functionReference = Default3DFunctions::CAMERA_PITCH_STICK;

    gamePadTrigger[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_GAMEPAD_AXIS_LEFT_TRIGGER;
    gamePadTrigger[GLInput::SINGLEKEYPRESS][0].functionReference = Default3DFunctions::CAMERA_FOV_TRIGGER;
    gamePadTrigger[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER;
    gamePadTrigger[GLInput::SINGLEKEYPRESS][1].functionReference = Default3DFunctions::CAMERA_SPEED_TRIGGER;

    // Keyboard Initialization
    keyboardHold[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_KEY_W;
    keyboardHold[GLInput::SINGLEKEYPRESS][0].functionReference = Default3DFunctions::MOVE_FORWARD_KEY;
    keyboardHold[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_KEY_S;
    keyboardHold[GLInput::SINGLEKEYPRESS][1].functionReference = Default3DFunctions::MOVE_BACKWARD_KEY;
    keyboardHold[GLInput::SINGLEKEYPRESS][2].glfwValue = GLFW_KEY_D;
    keyboardHold[GLInput::SINGLEKEYPRESS][2].functionReference = Default3DFunctions::MOVE_RIGHT_KEY;
    keyboardHold[GLInput::SINGLEKEYPRESS][3].glfwValue = GLFW_KEY_A;
    keyboardHold[GLInput::SINGLEKEYPRESS][3].functionReference = Default3DFunctions::MOVE_LEFT_KEY;
    keyboardHold[GLInput::SINGLEKEYPRESS][4].glfwValue = GLFW_KEY_UP;
    keyboardHold[GLInput::SINGLEKEYPRESS][4].functionReference = Default3DFunctions::CAMERA_PITCH_UP_KEY;
    keyboardHold[GLInput::SINGLEKEYPRESS][5].glfwValue = GLFW_KEY_DOWN;
    keyboardHold[GLInput::SINGLEKEYPRESS][5].functionReference = Default3DFunctions::CAMERA_PITCH_DOWN_KEY;
    keyboardHold[GLInput::SINGLEKEYPRESS][6].glfwValue = GLFW_KEY_RIGHT;
    keyboardHold[GLInput::SINGLEKEYPRESS][6].functionReference = Default3DFunctions::CAMERA_YAW_RIGHT_KEY;
    keyboardHold[GLInput::SINGLEKEYPRESS][7].glfwValue = GLFW_KEY_LEFT;
    keyboardHold[GLInput::SINGLEKEYPRESS][7].functionReference = Default3DFunctions::CAMERA_YAW_LEFT_KEY;

    keyboardClick[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_KEY_F;
    keyboardClick[GLInput::SINGLEKEYPRESS][0].functionReference = Default3DFunctions::TOGGLE_FLASHLIGHT;
    keyboardClick[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_KEY_F1;
    keyboardClick[GLInput::SINGLEKEYPRESS][1].functionReference = Default3DFunctions::TOGGLE_MENU;
    keyboardClick[GLInput::SINGLEKEYPRESS][2].glfwValue = GLFW_KEY_MINUS;
    keyboardClick[GLInput::SINGLEKEYPRESS][2].functionReference = Default3DFunctions::CAMERA_DEINCREMENT_SENSITIVITY;
    keyboardClick[GLInput::SINGLEKEYPRESS][3].glfwValue = GLFW_KEY_EQUAL;
    keyboardClick[GLInput::SINGLEKEYPRESS][3].functionReference = Default3DFunctions::CAMERA_INCREMENT_SENSITIVITY;
    keyboardClick[GLInput::SINGLEKEYPRESS][4].glfwValue = GLFW_KEY_LEFT_BRACKET;
    keyboardClick[GLInput::SINGLEKEYPRESS][4].functionReference = Default3DFunctions::CAMERA_DEINCREMENT_SPEED;
    keyboardClick[GLInput::SINGLEKEYPRESS][5].glfwValue = GLFW_KEY_RIGHT_BRACKET;
    keyboardClick[GLInput::SINGLEKEYPRESS][5].functionReference = Default3DFunctions::CAMERA_INCREMENT_SPEED;
    keyboardClick[GLInput::SINGLEKEYPRESS][6].glfwValue = GLFW_KEY_F9;
    keyboardClick[GLInput::SINGLEKEYPRESS][6].functionReference = Default3DFunctions::TOGGLE_DEVICE_MOUSE_MOVEMENT;

    // Mouse Scroll Initialization
    mouseScroll[GLInput::SINGLEKEYPRESS][0].glfwValue = 1;
    mouseScroll[GLInput::SINGLEKEYPRESS][0].functionReference = Default3DFunctions::CAMERA_FOV_MOUSE;


    // Mouse Movement Initialization
    mouseMovement[GLInput::SINGLEKEYPRESS][0].glfwValue = 1;
    mouseMovement[GLInput::SINGLEKEYPRESS][0].functionReference = Default3DFunctions::CAMERA_YAW_PITCH_MOUSE;


    this->default3DInput = new GLInput(isGamePadClick, isKeyboardHold, isMouseScroll,
        isGamePadStick, isKeyboardClick, isMouseMovement,
        isGamePadTrigger, gamePadClick, gamePadStick,
        gamePadTrigger, keyboardHold, keyboardClick,
        mouseScroll, mouseMovement, keyboardRelease);
    
    delete[] gamePadClick;
    delete[] gamePadStick;
    delete[] gamePadTrigger;
    delete[] keyboardHold;
    delete[] keyboardClick;
    delete[] mouseScroll;
    delete[] mouseMovement;
    delete[] keyboardRelease;

    
}

void InputManager::Create2DInput()
{
    // Toggle Device Input
    bool isGamePadClick = true;
    bool isGamePadStick = true;
    bool isGamePadTrigger = true;
    bool isKeyboardHold = true;
    bool isKeyboardClick = true;
    bool isMouseScroll = true;
    bool isMouseMovement = true;

    auto gamePadClick{ new GLClickLink[2][128] };
    auto gamePadStick{ new GLJoyStickLink[2][32] };
    auto gamePadTrigger{ new GLTriggerLink[2][8] };
    auto keyboardHold{ new GLHoldLink[2][128] };
    auto keyboardClick{ new GLClickLink[2][128] };
    auto mouseScroll{ new GLScrollLink[2][2] };
    auto mouseMovement{ new GLMouseMovementLink[2][2] };
    auto keyboardRelease{ new GLClickLink[2][128] };


    // GamePad Initialization
    gamePadClick[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_GAMEPAD_BUTTON_Y;
    gamePadClick[GLInput::SINGLEKEYPRESS][0].functionReference = Default2DFunctions::TOGGLE_FLASHLIGHT;
    gamePadClick[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_GAMEPAD_BUTTON_B;
    gamePadClick[GLInput::SINGLEKEYPRESS][1].functionReference = Default2DFunctions::RANDOM_FLASHLIGHT_COLOR;
    gamePadClick[GLInput::SINGLEKEYPRESS][2].glfwValue = GLFW_GAMEPAD_BUTTON_START;
    gamePadClick[GLInput::SINGLEKEYPRESS][2].functionReference = Default2DFunctions::TOGGLE_MENU;

    gamePadStick[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_GAMEPAD_AXIS_LEFT_X;
    gamePadStick[GLInput::SINGLEKEYPRESS][0].functionReference = Default2DFunctions::MOVE_LEFT_RIGHT_STICK_2D;
    gamePadStick[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_GAMEPAD_AXIS_LEFT_Y;
    gamePadStick[GLInput::SINGLEKEYPRESS][1].functionReference = Default2DFunctions::MOVE_UP_DOWN_STICK_2D;

    gamePadTrigger[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER;
    gamePadTrigger[GLInput::SINGLEKEYPRESS][0].functionReference = Default2DFunctions::CAMERA_SPEED_TRIGGER;

    // Keyboard Initialization
    keyboardHold[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_KEY_W;
    keyboardHold[GLInput::SINGLEKEYPRESS][0].functionReference = Default2DFunctions::MOVE_UP_2D;
    keyboardHold[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_KEY_S;
    keyboardHold[GLInput::SINGLEKEYPRESS][1].functionReference = Default2DFunctions::MOVE_DOWN_2D;
    keyboardHold[GLInput::SINGLEKEYPRESS][2].glfwValue = GLFW_KEY_D;
    keyboardHold[GLInput::SINGLEKEYPRESS][2].functionReference = Default2DFunctions::MOVE_RIGHT_2D;
    keyboardHold[GLInput::SINGLEKEYPRESS][3].glfwValue = GLFW_KEY_A;
    keyboardHold[GLInput::SINGLEKEYPRESS][3].functionReference = Default2DFunctions::MOVE_LEFT_2D;

    keyboardHold[GLInput::DOUBLEKEYPRESS][0].glfwValue = GLFW_KEY_W;
    keyboardHold[GLInput::DOUBLEKEYPRESS][0].glfwMask = GLFW_KEY_A;
    keyboardHold[GLInput::DOUBLEKEYPRESS][0].functionReference = Default2DFunctions::MOVE_UP_LEFT_2D;
    keyboardHold[GLInput::DOUBLEKEYPRESS][1].glfwValue = GLFW_KEY_W;
    keyboardHold[GLInput::DOUBLEKEYPRESS][1].glfwMask = GLFW_KEY_D;
    keyboardHold[GLInput::DOUBLEKEYPRESS][1].functionReference = Default2DFunctions::MOVE_UP_RIGHT_2D;
    keyboardHold[GLInput::DOUBLEKEYPRESS][2].glfwValue = GLFW_KEY_S;
    keyboardHold[GLInput::DOUBLEKEYPRESS][2].glfwMask = GLFW_KEY_A;
    keyboardHold[GLInput::DOUBLEKEYPRESS][2].functionReference = Default2DFunctions::MOVE_DOWN_LEFT_2D;
    keyboardHold[GLInput::DOUBLEKEYPRESS][3].glfwValue = GLFW_KEY_S;
    keyboardHold[GLInput::DOUBLEKEYPRESS][3].glfwMask = GLFW_KEY_D;
    keyboardHold[GLInput::DOUBLEKEYPRESS][3].functionReference = Default2DFunctions::MOVE_DOWN_RIGHT_2D;

    keyboardClick[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_KEY_F;
    keyboardClick[GLInput::SINGLEKEYPRESS][0].functionReference = Default2DFunctions::TOGGLE_FLASHLIGHT;
    keyboardClick[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_KEY_F1;
    keyboardClick[GLInput::SINGLEKEYPRESS][1].functionReference = Default2DFunctions::TOGGLE_MENU;
    keyboardClick[GLInput::SINGLEKEYPRESS][2].glfwValue = GLFW_KEY_MINUS;
    keyboardClick[GLInput::SINGLEKEYPRESS][2].functionReference = Default2DFunctions::CAMERA_DEINCREMENT_SENSITIVITY;
    keyboardClick[GLInput::SINGLEKEYPRESS][3].glfwValue = GLFW_KEY_EQUAL;
    keyboardClick[GLInput::SINGLEKEYPRESS][3].functionReference = Default2DFunctions::CAMERA_INCREMENT_SENSITIVITY;
    keyboardClick[GLInput::SINGLEKEYPRESS][4].glfwValue = GLFW_KEY_LEFT_BRACKET;
    keyboardClick[GLInput::SINGLEKEYPRESS][4].functionReference = Default2DFunctions::CAMERA_DEINCREMENT_SPEED;
    keyboardClick[GLInput::SINGLEKEYPRESS][5].glfwValue = GLFW_KEY_RIGHT_BRACKET;
    keyboardClick[GLInput::SINGLEKEYPRESS][5].functionReference = Default2DFunctions::CAMERA_INCREMENT_SPEED;
    keyboardClick[GLInput::SINGLEKEYPRESS][6].glfwValue = GLFW_KEY_F9;
    keyboardClick[GLInput::SINGLEKEYPRESS][6].functionReference = Default2DFunctions::TOGGLE_DEVICE_MOUSE_MOVEMENT;

    // Mouse Scroll Initialization
    mouseScroll[GLInput::SINGLEKEYPRESS][0].glfwValue = 1;
    mouseScroll[GLInput::SINGLEKEYPRESS][0].functionReference = Default2DFunctions::CAMERA_FOV_MOUSE;

    this->default2DInput = new GLInput(isGamePadClick, isKeyboardHold, isMouseScroll,
        isGamePadStick, isKeyboardClick, isMouseMovement,
        isGamePadTrigger, gamePadClick, gamePadStick,
        gamePadTrigger, keyboardHold, keyboardClick,
        mouseScroll, mouseMovement, keyboardRelease);

    delete[] gamePadClick;
    delete[] gamePadStick;
    delete[] gamePadTrigger;
    delete[] keyboardHold;
    delete[] keyboardClick;
    delete[] mouseScroll;
    delete[] mouseMovement;
    delete[] keyboardRelease;
}

void InputManager::CreateMenuInput()
{
    // Toggle Device Input
    bool isGamePadClick = true;
    bool isGamePadStick = true;
    bool isGamePadTrigger = true;
    bool isKeyboardHold = true;
    bool isKeyboardClick = true;
    bool isMouseScroll = true;
    bool isMouseMovement = true;

    auto gamePadClick{ new GLClickLink[2][128] };
    auto gamePadStick{ new GLJoyStickLink[2][32] };
    auto gamePadTrigger{ new GLTriggerLink[2][8] };
    auto keyboardHold{ new GLHoldLink[2][128] };
    auto keyboardClick{ new GLClickLink[2][128] };
    auto mouseScroll{ new GLScrollLink[2][2] };
    auto mouseMovement{ new GLMouseMovementLink[2][2] };
    auto keyboardRelease{ new GLClickLink[2][128] };


    // GamePad Initialization
    gamePadClick[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_GAMEPAD_BUTTON_START;
    gamePadClick[GLInput::SINGLEKEYPRESS][0].functionReference = DebugMenuFunctions::TOGGLE_MENU;
    gamePadClick[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_GAMEPAD_BUTTON_B;
    gamePadClick[GLInput::SINGLEKEYPRESS][1].functionReference = DebugMenuFunctions::CLOSE_CURRENT_WINDOW;

    // Keyboard Initialization

    keyboardClick[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_KEY_F1;
    keyboardClick[GLInput::SINGLEKEYPRESS][0].functionReference = DebugMenuFunctions::TOGGLE_MENU;
    keyboardClick[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_KEY_TAB;
    keyboardClick[GLInput::SINGLEKEYPRESS][1].functionReference = DebugMenuFunctions::CLOSE_CURRENT_WINDOW;

    // Mouse Scroll Initialization

    // Mouse Movement Initialization

    this->debugMenuInput = new GLInput(isGamePadClick, isKeyboardHold, isMouseScroll,
        isGamePadStick, isKeyboardClick, isMouseMovement,
        isGamePadTrigger, gamePadClick, gamePadStick,
        gamePadTrigger, keyboardHold, keyboardClick,
        mouseScroll, mouseMovement, keyboardRelease);


    delete[] gamePadClick;
    delete[] gamePadStick;
    delete[] gamePadTrigger;
    delete[] keyboardHold;
    delete[] keyboardClick;
    delete[] mouseScroll;
    delete[] mouseMovement;
    delete[] keyboardRelease;
}

 /*============================================================================================================================================================================================
 * External Functionality
 *============================================================================================================================================================================================*/




