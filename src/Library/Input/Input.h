#pragma once
#include "../Includes.h"

/*============================================================================================================================================================================================
 * Function Enum References
 *============================================================================================================================================================================================*/

enum FunctionReferences
{
	// Click Input
	TOGGLE_FLASHLIGHT, EXIT_APPLICATION, RANDOM_FLASHLIGHT_COLOR, TOGGLE_MENU,
	CAMERA_INCREMENT_SENSITIVITY, CAMERA_DEINCREMENT_SENSITIVITY, CAMERA_INCREMENT_SPEED, CAMERA_DEINCREMENT_SPEED,
	TOGGLE_DEVICE_MOUSE_MOVEMENT, CLOSE_CURRENT_WINDOW,

	// Keyboard Hold Input
	//3D
	MOVE_FORWARD_KEY, MOVE_BACKWARD_KEY, MOVE_LEFT_KEY, MOVE_RIGHT_KEY,
	CAMERA_YAW_RIGHT_KEY, CAMERA_YAW_LEFT_KEY, CAMERA_PITCH_UP_KEY, CAMERA_PITCH_DOWN_KEY,
	//2D
	MOVE_UP_2D, MOVE_DOWN_2D, MOVE_LEFT_2D, MOVE_RIGHT_2D, MOVE_UP_LEFT_2D, MOVE_UP_RIGHT_2D, MOVE_DOWN_LEFT_2D, MOVE_DOWN_RIGHT_2D,
	
	// Mouse Input
	CAMERA_YAW_PITCH_MOUSE,
	// Mouse Scroll Input
	CAMERA_FOV_MOUSE,

	// GamePad JoyStick
	CAMERA_YAW_STICK, CAMERA_PITCH_STICK, MOVE_LEFT_RIGHT_STICK, MOVE_UP_DOWN_STICK,
	MOVE_LEFT_RIGHT_STICK_2D, MOVE_UP_DOWN_STICK_2D,

	// GamePad Trigger
	CAMERA_FOV_TRIGGER, CAMERA_SPEED_TRIGGER,

	// Default
	NA

};

/*============================================================================================================================================================================================
 * GLButton Class
 *============================================================================================================================================================================================*/


class GLButton
{
public:
	int previousState;
	int glfwValue;
	int glfwMask;
	FunctionReferences function;
public:
	GLButton();
};


/*============================================================================================================================================================================================
 * GLInput Class
 *============================================================================================================================================================================================*/
class GLInput
{
public:
	enum MaskNumber 
	{
		SINGLEKEYPRESS = 0, DOUBLEKEYPRESS = 1, TRIPLEKEYPRESS = 2
	};
public:
	GLButton gamePadClick[2][128];
	int gamePadClickSize[2] = {0, 0};
	GLButton gamePadStick[2][32];
	int gamePadStickSize[2] = { 0, 0 };
	GLButton gamePadTrigger[2][8];
	int gamePadTriggerSize[2] = { 0, 0 };
	GLButton keyboardHold[2][128];
	int keyboardHoldSize[2] = { 0, 0 };
	GLButton keyboardClick[2][128];
	int keyboardClickSize[2] = { 0, 0 };
	GLButton mouseScroll[2][2];
	int mouseScrollSize[2] = { 0, 0 };
	GLButton mouseMovement[2][2];
	int mouseMovementSize[2] = { 0, 0 };
public:
public:
	bool isGamePadClick, isGamePadStick, isGamePadTrigger;
	bool isKeyboardHold, isKeyboardClick;
	bool isMouseScroll, isMouseMovement;
public:
	void processScroll(double xoffset, double yoffset);
	void processMouseMovement(double xpos, double ypos);
	void processGamePad();
	void processGamePadStick(const float* axes);
	void processGamePadTrigger(const float* axes);
	void processGamePadClick();
	void processKeyboardHold(GLFWwindow* window);
	void processKeyboardClick(GLFWwindow* window, int key, int scancode, int action, int mods);
};

/*============================================================================================================================================================================================
 * InputManager Class
 *============================================================================================================================================================================================*/
class InputManager
{
public:
	GLInput default3DInput;
	GLInput debugMenuInput;
	GLInput default2DInput;
	FunctionReferences functions[1024];
public:
	bool accessClickFunction(FunctionReferences index, GLButton* gamePadButton);
	bool accessHoldFunctions(FunctionReferences index);
	void accessScrollFunctions(FunctionReferences index, double xoffset, double yoffset);
	void accessMouseMovementFunctions(FunctionReferences index, double xpos, double ypos);
	bool accessJoyStickFunctions(FunctionReferences index, float axis);
	void accessTriggerFunctions(FunctionReferences index, float axis);
public:
	void CreateInputs();
	void DestroyInputs();
private:
	void Create3DInput(GLInput& input);
	void Create2DInput(GLInput& input);
	void CreateMenuInput(GLInput& input);
	void DestroyInput(GLInput& input);

};