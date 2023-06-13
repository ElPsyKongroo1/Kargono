#pragma once
#include "../../Rendering/Includes.h"

/*============================================================================================================================================================================================
 * GLButton Class
 *============================================================================================================================================================================================*/


class GLInputLink
{
public:
	int previousState;
	int glfwValue;
	int glfwMask;
public:
	GLInputLink()
	{
		previousState = GLFW_RELEASE;
		glfwValue = -1;
	}
};

class GLMouseMovementLink : public GLInputLink
{
public:
	void (*functionReference)(double, double);
public:
	GLMouseMovementLink() : GLInputLink(), functionReference{nullptr}
	{
			
	}
};

class GLScrollLink : public GLInputLink
{
public:
	void (*functionReference)(double, double);
public:
	GLScrollLink() : GLInputLink(), functionReference{ nullptr }
	{

	}
};

class GLClickLink : public GLInputLink
{
public:
	bool (*functionReference)(GLInputLink*);
public:
	GLClickLink() : GLInputLink(), functionReference{ nullptr }
	{

	}
};

class GLHoldLink : public GLInputLink
{
public:
	bool (*functionReference)();
public:
	GLHoldLink() : GLInputLink(), functionReference{ nullptr }
	{

	}
};

class GLTriggerLink : public GLInputLink
{
public:
	void (*functionReference)(float);
public:
	GLTriggerLink() : GLInputLink(), functionReference{ nullptr }
	{

	}
};

class GLJoyStickLink : public GLInputLink
{
public:
	bool (*functionReference)(float);
public:
	GLJoyStickLink() : GLInputLink(), functionReference{ nullptr }
	{

	}
};

/*============================================================================================================================================================================================
 * GLInput Class
 *============================================================================================================================================================================================*/
class GLInput
{
public:
	GLInput() {};
	GLInput(bool valPadClick, bool valKeyHold, bool valMouseScroll,
		bool valPadStick, bool valKeyClick, bool valMouseMove,
		bool valPadTrigger, GLClickLink gamePadClick[2][128],
		GLJoyStickLink gamePadStick[2][32], GLTriggerLink gamePadTrigger[2][8],
		GLHoldLink keyboardHold[2][128], GLClickLink keyboardClick[2][128],
		GLScrollLink mouseScroll[2][2], GLMouseMovementLink mouseMovement[2][2]);
	~GLInput();
public:
	enum MaskNumber 
	{
		SINGLEKEYPRESS = 0, DOUBLEKEYPRESS = 1, TRIPLEKEYPRESS = 2
	};
public:
	GLClickLink gamePadClick[2][128];
	int gamePadClickSize[2] = {0, 0};
	GLJoyStickLink gamePadStick[2][32];
	int gamePadStickSize[2] = { 0, 0 };
	GLTriggerLink gamePadTrigger[2][8];
	int gamePadTriggerSize[2] = { 0, 0 };
	GLHoldLink keyboardHold[2][128];
	int keyboardHoldSize[2] = { 0, 0 };
	GLClickLink keyboardClick[2][128];
	int keyboardClickSize[2] = { 0, 0 };
	GLScrollLink mouseScroll[2][2];
	int mouseScrollSize[2] = { 0, 0 };
	GLMouseMovementLink mouseMovement[2][2];
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
	InputManager() {};
public:
	GLInput* default3DInput;
	GLInput* debugMenuInput;
	GLInput* default2DInput;
public:
	void CreateInputs();
	void DestroyInputs();
private:
	void Create3DInput();
	void Create2DInput();
	void CreateMenuInput();

};