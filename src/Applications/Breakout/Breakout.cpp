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
void processBrickCollisions(GameBall* ball, std::vector<GameBrick*> bricks, GameLevel* level);
void processPaddleCollision(GameBall* ball, GamePaddle* paddle, GameLevel* level);

void BreakoutStart()
{
	// Initialize GLFW context, Meshes, Shaders, and Textures
	initializeRenderer();
	Resources::currentGame->resourceManager = new ResourceManager();
	Resources::currentGame->resourceManager->initializeResources();
	GameLevel* currentLevel{ new GameLevel(25, 18) };
	currentLevel->Load("Resources/Breakout/Map/Level1.txt");
    Resources::currentGame->currentLevel = currentLevel;
	for (GameBrick* object : currentLevel->currentMapBricks)
	{
		if (!object) { continue; }
		Resources::currentApplication->renderer->objectRenderBuffer.push_back(object);
	}

	Orientation orientation = { glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
				  glm::vec3(0.0f, -250.0f, 0.0f),
				  glm::vec3(60.0f, 60.0f, 0.0f) };
	ShapeRenderer* renderer = { new ShapeRenderer(orientation,
		Resources::currentGame->resourceManager->applicationMeshes.at(3),
		Resources::currentApplication->renderer->defaultShader) };
	GamePaddle* paddle{ new GamePaddle(orientation, renderer) };
	Resources::currentApplication->renderer->objectRenderBuffer.push_back(paddle);
    Resources::currentGame->paddle = paddle;
    paddle->objectSpeed = 200.0f;

    Orientation orientationBall = { glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
                  glm::vec3(0.0f, 0.0f, 0.0f),
                  glm::vec3(15.0f, 15.0f, 0.0f) };
    ShapeRenderer* rendererBall = { new ShapeRenderer(orientation,
        Resources::currentGame->resourceManager->applicationMeshes.at(2),
        Resources::currentApplication->renderer->defaultShader) };
    GameBall* ball{ new GameBall(orientationBall, rendererBall) };
    Resources::currentGame->ball = ball;
    
    Resources::currentApplication->renderer->objectRenderBuffer.push_back(ball);
    ball->objectSpeed = 150.0f;

    //currentLevel.RemoveBrick(currentLevel.currentMapBricks.at(0)); FIXME
	


	// Main running Loop
	while (!glfwWindowShouldClose(Resources::currentApplication->renderer->window))
	{
        //ball->Move();
        processBrickCollisions(ball, currentLevel->currentMapBricks, currentLevel);
        processPaddleCollision(ball, paddle, currentLevel);
		Resources::currentApplication->renderer->render();

	}


    Resources::currentGame->paddle = nullptr;
	delete paddle;
	paddle = nullptr;

    delete ball;
    ball = nullptr;

    delete currentLevel;
    currentLevel = nullptr;

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
		glm::vec2(-400.0f, 400.0f), glm::vec2(-300.0f, 300.0f), glm::vec2(0.1f, 300.0f), 45.0f,
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
    keyboardHold[GLInput::SINGLEKEYPRESS][2].glfwValue = GLFW_KEY_UP;
    keyboardHold[GLInput::SINGLEKEYPRESS][2].functionReference = BreakoutInputFunctions::MOVE_BALL_UP;
    keyboardHold[GLInput::SINGLEKEYPRESS][3].glfwValue = GLFW_KEY_DOWN;
    keyboardHold[GLInput::SINGLEKEYPRESS][3].functionReference = BreakoutInputFunctions::MOVE_BALL_DOWN;
    keyboardHold[GLInput::SINGLEKEYPRESS][4].glfwValue = GLFW_KEY_LEFT;
    keyboardHold[GLInput::SINGLEKEYPRESS][4].functionReference = BreakoutInputFunctions::MOVE_BALL_LEFT;
    keyboardHold[GLInput::SINGLEKEYPRESS][5].glfwValue = GLFW_KEY_RIGHT;
    keyboardHold[GLInput::SINGLEKEYPRESS][5].functionReference = BreakoutInputFunctions::MOVE_BALL_RIGHT;
    keyboardHold[GLInput::SINGLEKEYPRESS][6].glfwValue = GLFW_KEY_EQUAL;
    keyboardHold[GLInput::SINGLEKEYPRESS][6].functionReference = BreakoutInputFunctions::INCREMENT_BALL_SPEED;
    keyboardHold[GLInput::SINGLEKEYPRESS][7].glfwValue = GLFW_KEY_MINUS;
    keyboardHold[GLInput::SINGLEKEYPRESS][7].functionReference = BreakoutInputFunctions::DEINCREMENT_BALL_SPEED;


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
void processPaddleCollision(GameBall* ball, GamePaddle* paddle, GameLevel* level)
{
    if (glm::distance(paddle->orientation.translation, ball->orientation.translation) < (ball->Radius + glm::length(paddle->currentDimensions)))
    {
        //std::cout << "Paddle Collision Entered " << "Here are the dimensions of the paddle" << "Width: " << paddle->currentDimensions.x << ' ' << paddle->currentDimensions.y << '\n';
        std::cout << "Paddle Collision Entered " << "Here are the dimensions of the paddle" << "Width: " << paddle->currentDimensions.x / paddle->orientation.scale.x << ' ' << paddle->currentDimensions.y << '\n';
        enum ballRelativeLocation { LEFT, BELOW, ABOVE, RIGHT };
        ballRelativeLocation currentRelativeLocation;
        // y=mx+b for line between ball and brick
        bool is_delta_x_0 = false;
        float delta_y = paddle->orientation.translation.y - ball->orientation.translation.y;
        float delta_x = paddle->orientation.translation.x - ball->orientation.translation.x;

        float slope;
        float y_intercept;
        if (delta_x == 0)
        {
            is_delta_x_0 = true;
            slope = 1000000;
            y_intercept = ball->orientation.translation.y - (slope * ball->orientation.translation.x);
        }

        else
        {
            slope = delta_y / delta_x;
            y_intercept = ball->orientation.translation.y - (slope * ball->orientation.translation.x);
        }

        if (slope <= -1 || slope >= 1 || is_delta_x_0)
        {
            if (ball->orientation.translation.y > paddle->orientation.translation.y)
            {
                currentRelativeLocation = ABOVE;
            }
            else
            {
                currentRelativeLocation = BELOW;
            }
        }
        else
        {
            if (ball->orientation.translation.x > paddle->orientation.translation.x)
            {
                currentRelativeLocation = RIGHT;
            }
            else
            {
                currentRelativeLocation = LEFT;
            }
        }
        // Calculate Closest Point on Brick to Circle
        glm::vec2 closestPointOnBrickToBall = glm::vec2();
        switch (currentRelativeLocation)
        {
        case ABOVE:
            closestPointOnBrickToBall.y = paddle->orientation.translation.y + (0.5f * paddle->currentDimensions.y);
            closestPointOnBrickToBall.x = (closestPointOnBrickToBall.y - y_intercept) / slope;
            break;
        case BELOW:
            closestPointOnBrickToBall.y = paddle->orientation.translation.y - (0.5f * paddle->currentDimensions.y);
            closestPointOnBrickToBall.x = (closestPointOnBrickToBall.y - y_intercept) / slope;
            break;
        case LEFT:
            closestPointOnBrickToBall.x = paddle->orientation.translation.x - (0.5f * paddle->currentDimensions.x);
            closestPointOnBrickToBall.y = (slope * closestPointOnBrickToBall.x) + y_intercept;
            break;
        case RIGHT:
            closestPointOnBrickToBall.x = paddle->orientation.translation.x + (0.5f * paddle->currentDimensions.x);
            closestPointOnBrickToBall.y = (slope * closestPointOnBrickToBall.x) + y_intercept;
            break;
        default:
            std::cerr << "Invalid enumeration in Breakout.cpp::processBrickCollision::CalcClosestCircle\n";
            break;
        }
        //switch (currentRelativeLocation) 
        //{
        //case ABOVE:
        //    std::cout << "We are registering ABOVE!\n";
        //    break;
        //case BELOW:
        //    std::cout << "We are registering BELOW!\n";
        //    break;
        //case LEFT:
        //    std::cout << "We are registering LEFT!\n";
        //    break;
        //case RIGHT:
        //    std::cout << "We are registering RIGHT!\n";
        //    break;
        //}
        std::cout << "This is the paddle's current location: " << paddle->orientation.translation.x << paddle->orientation.translation.y << '\n';
        std::cout << "This is the closeset point: " << closestPointOnBrickToBall.x << closestPointOnBrickToBall.y << '\n';
        std::cout << "Current Location of ball: " << ball->orientation.translation.x << ball->orientation.translation.y << '\n';

        if (glm::distance(closestPointOnBrickToBall, glm::vec2(ball->orientation.translation)) < ball->Radius)
        {
            std::cout << "FULL COLLISION OCCURING\n";
            switch (currentRelativeLocation)
            {
            case ABOVE:
                ball->orientation.translation.y = paddle->orientation.translation.y +
                    (paddle->currentDimensions.y / 2) +
                    ball->Radius + 0.1f;
                ball->direction.y = -ball->direction.y;
                break;
            case BELOW:
                ball->orientation.translation.y = paddle->orientation.translation.y -
                    (paddle->currentDimensions.y / 2) -
                    ball->Radius - 0.1f;
                ball->direction.y = -ball->direction.y;
                break;
            case LEFT:
                ball->orientation.translation.x = paddle->orientation.translation.x -
                    (paddle->currentDimensions.x / 2) -
                    ball->Radius - 0.1f;
                ball->direction.x = -ball->direction.x;
                break;
            case RIGHT:
                ball->orientation.translation.x = paddle->orientation.translation.x +
                    (paddle->currentDimensions.x / 2) +
                    ball->Radius + 0.1f;
                ball->direction.x = -ball->direction.x;
                break;
            default:
                std::cerr << "Invalid enumeration in Breakout.cpp::processBrickCollision::ChangeCircleDirection\n";
                break;
            }
        }

    }
}


void processBrickCollisions(GameBall* ball, std::vector<GameBrick*> bricks, GameLevel* level) 
{
    for (GameBrick* brick : bricks) 
    {
        if (glm::distance(brick->orientation.translation, ball->orientation.translation) < (ball->Radius + glm::length(brick->currentDimensions))) 
        {
            enum ballRelativeLocation{ LEFT, BELOW, ABOVE, RIGHT };
            ballRelativeLocation currentRelativeLocation;
            // y=mx+b for line between ball and brick
            bool is_delta_x_0 = false;
            float delta_y = brick->orientation.translation.y - ball->orientation.translation.y;
            float delta_x = brick->orientation.translation.x - ball->orientation.translation.x;
            
            float slope;
            float y_intercept;
            if (delta_x == 0) 
            {
                is_delta_x_0 = true;
                slope = 0;
                y_intercept = 0;
            }
            else 
            {
                slope = delta_y / delta_x;
                y_intercept = ball->orientation.translation.y - (slope * ball->orientation.translation.x);
            }

            if (slope <= -1 || slope >= 1 || is_delta_x_0)
            {
                if (ball->orientation.translation.y > brick->orientation.translation.y)
                {
                    currentRelativeLocation = ABOVE;
                }
                else 
                {
                    currentRelativeLocation = BELOW;
                }
            }
            else 
            {
                if (ball->orientation.translation.x > brick->orientation.translation.x)
                {
                    currentRelativeLocation = RIGHT;
                }
                else 
                {
                    currentRelativeLocation = LEFT;
                }
            }
            // Calculate Closest Point on Brick to Circle
            glm::vec2 closestPointOnBrickToBall = glm::vec2();
            switch (currentRelativeLocation) 
            {
            case ABOVE:
                closestPointOnBrickToBall.y = brick->orientation.translation.y + (0.5f * brick->currentDimensions.y);
                closestPointOnBrickToBall.x = (closestPointOnBrickToBall.y - y_intercept) / slope;
                break;
            case BELOW:
                closestPointOnBrickToBall.y = brick->orientation.translation.y - (0.5f * brick->currentDimensions.y);
                closestPointOnBrickToBall.x = (closestPointOnBrickToBall.y - y_intercept) / slope;
                break;
            case LEFT:
                closestPointOnBrickToBall.x = brick->orientation.translation.x - (0.5f * brick->currentDimensions.x);
                closestPointOnBrickToBall.y = (slope * closestPointOnBrickToBall.x) + y_intercept;
                break;
            case RIGHT:
                closestPointOnBrickToBall.x = brick->orientation.translation.x + (0.5f * brick->currentDimensions.x);
                closestPointOnBrickToBall.y = (slope * closestPointOnBrickToBall.x) + y_intercept;
                break;
            default:
                std::cerr << "Invalid enumeration in Breakout.cpp::processBrickCollision::CalcClosestCircle\n";
                break;
            }
            if (glm::distance(closestPointOnBrickToBall, glm::vec2(ball->orientation.translation)) < ball->Radius) 
            {
                switch (currentRelativeLocation) 
                {
                case ABOVE:
                    ball->orientation.translation.y = brick->orientation.translation.y + 
                                                      (brick->currentDimensions.y / 2) + 
                                                      ball->Radius + 0.1f;
                    ball->direction.y = -ball->direction.y;
                    break;
                case BELOW:
                    ball->orientation.translation.y = brick->orientation.translation.y -
                        (brick->currentDimensions.y / 2) -
                        ball->Radius - 0.1f;
                    ball->direction.y = -ball->direction.y;
                    break;
                case LEFT:
                    ball->orientation.translation.x = brick->orientation.translation.x -
                        (brick->currentDimensions.x / 2) -
                        ball->Radius - 0.1f;
                    ball->direction.x = -ball->direction.x;
                    break;
                case RIGHT:
                    ball->orientation.translation.x = brick->orientation.translation.x +
                        (brick->currentDimensions.x / 2) +
                        ball->Radius + 0.1f;
                    ball->direction.x = -ball->direction.x;
                    break;
                default:
                    std::cerr << "Invalid enumeration in Breakout.cpp::processBrickCollision::ChangeCircleDirection\n";
                    break;
                }
                level->RemoveBrick(brick);
            }
            
        }
    }
   
    
}