#include "Breakout.h"
#include "../../Library/Includes.h"

#include "../../Library/Library.h"
#include "../../Library/Rendering/Shaders/Shaders.h"
#include "../../Library/Rendering/Mesh/Meshes.h"
#include "../../Library/Rendering/Textures/Textures.h"
#include "../../Library/Application/Objects/Objects.h"
#include "Classes/ResourceManager.h"
#include "Classes/GameObject.h"
#include "Classes/GameLevel.h"
#include "Classes/BreakoutInputFunctions.h"
void initializeRenderer();
void CreateBreakoutInput();

void BreakoutStart()
{
	// Initialize GLFW context, Meshes, Shaders, and Textures
	initializeRenderer();
	Resources::currentGame->resourceManager = new ResourceManager();
	Resources::currentGame->resourceManager->initializeResources();
	GameLevel currentLevel{ GameLevel(25, 18) };
	currentLevel.Load("Resources/Breakout/Map/Level1.txt");
	for (GameObject* object : currentLevel.currentMap)
	{
		if (!object) { continue; }
		Resources::currentApplication->renderer->objectRenderBuffer.push_back(object);
	}

	Orientation orientation = { glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
				  glm::vec3(0.0f, -250.0f, 0.3f),
				  glm::vec3(60.0f, 60.0f, 0.0f) };
	ShapeRenderer* renderer = { new ShapeRenderer(orientation,
		Resources::currentGame->resourceManager->applicationMeshes.at(3),
		Resources::currentApplication->renderer->defaultShader) };
	GameObject* paddle{ new GameObject(orientation, renderer) };
	Resources::currentApplication->renderer->objectRenderBuffer.push_back(paddle);
    Resources::currentGame->focusedObject = paddle;
    paddle->objectSpeed = 200.0f;

    Orientation orientationBall = { glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
                  glm::vec3(0.0f, -200.0f, 0.0f),
                  glm::vec3(15.0f, 15.0f, 0.0f) };
    ShapeRenderer* rendererBall = { new ShapeRenderer(orientation,
        Resources::currentGame->resourceManager->applicationMeshes.at(2),
        Resources::currentApplication->renderer->defaultShader) };
    GameBall* ball{ new GameBall(orientationBall, rendererBall) };
    Resources::currentApplication->renderer->objectRenderBuffer.push_back(ball);
    ball->objectSpeed = 150.0f;

	
	


	// Main running Loop
	while (!glfwWindowShouldClose(Resources::currentApplication->renderer->window))
	{
        ball->Move();
		Resources::currentApplication->renderer->render();
	}


    Resources::currentGame->focusedObject = nullptr;
	delete paddle;
	paddle = nullptr;

    delete ball;
    ball = nullptr;

    delete Resources::currentApplication->defaultInput;
    Resources::currentApplication->currentInput = nullptr;
    Resources::currentApplication->defaultInput = nullptr;

	delete Resources::currentGame->resourceManager;
	Resources::currentGame->resourceManager = nullptr;
	delete Resources::currentApplication->renderer->currentCamera;
	Resources::currentApplication->renderer->currentCamera = nullptr;
	Resources::currentApplication->renderer->close();
}


void initializeRenderer() 
{
    Resources::currentApplication->renderer->init();
    CreateBreakoutInput();
	Resources::currentApplication->renderer->currentCamera = new GLCamera(glm::vec3(0.0f, 0.0f, 50.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(-90.0f, 0.0f, 0.0f), 3.0f, GLCamera::ORTHOGRAPHIC,
		glm::vec2(-400.0f, 400.0f), glm::vec2(-300.0f, 300.0f), glm::vec2(0.1f, 100.0f), 45.0f,
		(float)Resources::currentApplication->renderer->screenDimension.x / (float)Resources::currentApplication->renderer->screenDimension.y,
		0.1f);
    
    Resources::currentApplication->renderer->setDefaultValues(Resources::modelManager.simpleBackpack,
        Resources::meshManager.cubeMesh,
        Resources::shaderManager.defaultShader,
		Resources::currentApplication->renderer->currentCamera);
	
}

void CreateBreakoutInput()
{
    // Toggle Device Input
    bool isGamePadClick = false;
    bool isGamePadStick = false;
    bool isGamePadTrigger = false;
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

    // GamePad Initialization
    gamePadClick[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_GAMEPAD_BUTTON_Y;
    gamePadClick[GLInput::SINGLEKEYPRESS][0].functionReference = BreakoutInputFunctions::TOGGLE_FLASHLIGHT;
    gamePadClick[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_GAMEPAD_BUTTON_B;
    gamePadClick[GLInput::SINGLEKEYPRESS][1].functionReference = BreakoutInputFunctions::RANDOM_FLASHLIGHT_COLOR;
    gamePadClick[GLInput::SINGLEKEYPRESS][2].glfwValue = GLFW_GAMEPAD_BUTTON_START;
    gamePadClick[GLInput::SINGLEKEYPRESS][2].functionReference = BreakoutInputFunctions::TOGGLE_MENU;

    gamePadStick[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_GAMEPAD_AXIS_LEFT_X;
    gamePadStick[GLInput::SINGLEKEYPRESS][0].functionReference = BreakoutInputFunctions::MOVE_LEFT_RIGHT_STICK_2D;
    gamePadStick[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_GAMEPAD_AXIS_LEFT_Y;
    gamePadStick[GLInput::SINGLEKEYPRESS][1].functionReference = BreakoutInputFunctions::MOVE_UP_DOWN_STICK_2D;

    gamePadTrigger[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER;
    gamePadTrigger[GLInput::SINGLEKEYPRESS][0].functionReference = BreakoutInputFunctions::CAMERA_SPEED_TRIGGER;

    // Keyboard Initialization
    keyboardHold[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_KEY_D;
    keyboardHold[GLInput::SINGLEKEYPRESS][0].functionReference = BreakoutInputFunctions::MOVE_RIGHT_2D;
    keyboardHold[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_KEY_A;
    keyboardHold[GLInput::SINGLEKEYPRESS][1].functionReference = BreakoutInputFunctions::MOVE_LEFT_2D;

    // Double Key Press Location

    keyboardClick[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_KEY_F;
    keyboardClick[GLInput::SINGLEKEYPRESS][0].functionReference = BreakoutInputFunctions::TOGGLE_FLASHLIGHT;
    keyboardClick[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_KEY_F1;
    keyboardClick[GLInput::SINGLEKEYPRESS][1].functionReference = BreakoutInputFunctions::TOGGLE_MENU;
    keyboardClick[GLInput::SINGLEKEYPRESS][2].glfwValue = GLFW_KEY_MINUS;
    keyboardClick[GLInput::SINGLEKEYPRESS][2].functionReference = BreakoutInputFunctions::CAMERA_DEINCREMENT_SENSITIVITY;
    keyboardClick[GLInput::SINGLEKEYPRESS][3].glfwValue = GLFW_KEY_EQUAL;
    keyboardClick[GLInput::SINGLEKEYPRESS][3].functionReference = BreakoutInputFunctions::CAMERA_INCREMENT_SENSITIVITY;
    keyboardClick[GLInput::SINGLEKEYPRESS][4].glfwValue = GLFW_KEY_LEFT_BRACKET;
    keyboardClick[GLInput::SINGLEKEYPRESS][4].functionReference = BreakoutInputFunctions::CAMERA_DEINCREMENT_SPEED;
    keyboardClick[GLInput::SINGLEKEYPRESS][5].glfwValue = GLFW_KEY_RIGHT_BRACKET;
    keyboardClick[GLInput::SINGLEKEYPRESS][5].functionReference = BreakoutInputFunctions::CAMERA_INCREMENT_SPEED;
    keyboardClick[GLInput::SINGLEKEYPRESS][6].glfwValue = GLFW_KEY_F9;
    keyboardClick[GLInput::SINGLEKEYPRESS][6].functionReference = BreakoutInputFunctions::TOGGLE_DEVICE_MOUSE_MOVEMENT;

    // Mouse Scroll Initialization
    mouseScroll[GLInput::SINGLEKEYPRESS][0].glfwValue = 1;
    mouseScroll[GLInput::SINGLEKEYPRESS][0].functionReference = BreakoutInputFunctions::CAMERA_FOV_MOUSE;

    

    Resources::currentApplication->defaultInput = new GLInput(isGamePadClick, isKeyboardHold, isMouseScroll,
                                                              isGamePadStick, isKeyboardClick, isMouseMovement,
                                                              isGamePadTrigger, gamePadClick, gamePadStick,
                                                              gamePadTrigger, keyboardHold, keyboardClick,
                                                              mouseScroll, mouseMovement);
    Resources::currentApplication->currentInput = Resources::currentApplication->defaultInput;

    delete[] gamePadClick;
    delete[] gamePadStick;
    delete[] gamePadTrigger;
    delete[] keyboardHold;
    delete[] keyboardClick;
    delete[] mouseScroll;
    delete[] mouseMovement;
}