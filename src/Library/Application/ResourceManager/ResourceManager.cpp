#include "ResourceManager.h"
#include "../../../Applications/Breakout/Classes/InputFunctions/BreakoutActiveFunctions.h"
#include "../../../Applications/Breakout/Classes/InputFunctions/BreakoutStartFunctions.h"

void ResourceManager::initializeResources() 
{
	Texture* purpleBlock = new Texture("Resources/Breakout/Textures/Original/PurpleBlock.png", "texture_diffuse");
	Texture* blueBlock = new Texture("Resources/Breakout/Textures/Original/BlueBlock.png", "texture_diffuse");
	Texture* specBlock = new Texture("Resources/Breakout/Textures/SpecularMap/SpecularBlock.png", "texture_specular");
	Texture* basePaddle = new Texture("Resources/Breakout/Textures/Original/BasePaddle.png", "texture_diffuse");
	Texture* specPaddle = new Texture("Resources/Breakout/Textures/SpecularMap/specPaddle.png", "texture_specular");
    Texture* baseBall = new Texture("Resources/Breakout/Textures/Original/BaseBall.png", "texture_diffuse");
    Texture* specBall = new Texture("Resources/Breakout/Textures/SpecularMap/specBall.png", "texture_specular");


    this->localTextures.push_back(purpleBlock);
    this->localTextures.push_back(specBlock);

    glm::vec3 dimensions = glm::vec3(0.5f, 0.5f, 0.0f);

    std::vector<unsigned int> indices {std::vector<unsigned int>()};
    std::vector<Texture*> textures {std::vector<Texture*>()};
    textures.push_back(purpleBlock);
    textures.push_back(specBlock);
    std::vector<Vertex> vertices
    {
            Vertex(glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)),
            Vertex(glm::vec3(0.5f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
            Vertex(glm::vec3(0.5f, 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)),
            Vertex(glm::vec3(0.5f, 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)),
            Vertex(glm::vec3(-0.5f, 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)),
            Vertex(glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f))
    };
    GLMesh* purpleMesh{ new GLMesh(vertices, indices, textures, GLMesh::DRAWARRAYS, dimensions) };
    localMeshes.push_back(purpleMesh);

    textures.clear();
    textures.push_back(blueBlock);
    textures.push_back(specBlock);
    GLMesh* blueMesh{ new GLMesh(vertices, indices, textures, GLMesh::DRAWARRAYS, dimensions) };
    localMeshes.push_back(blueMesh);


    textures.clear();
    textures.push_back(baseBall);
    textures.push_back(specBall);

    GLMesh* ballMesh{ new GLMesh(vertices, indices, textures, GLMesh::DRAWARRAYS, dimensions) };
    localMeshes.push_back(ballMesh);


    vertices = 
    {
            Vertex(glm::vec3(-0.5f, -0.125f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)),
            Vertex(glm::vec3(0.5f, -0.125f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)),
            Vertex(glm::vec3(0.5f, 0.125f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)),
            Vertex(glm::vec3(0.5f, 0.125f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)),
            Vertex(glm::vec3(-0.5f, 0.125f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)),
            Vertex(glm::vec3(-0.5f, -0.125f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f))
    };
    textures.clear();
    textures.push_back(basePaddle);
    textures.push_back(specPaddle);
    dimensions = glm::vec3(0.5f, 0.125f, 0.0f);

    GLMesh* paddleMesh{ new GLMesh(vertices, indices, textures, GLMesh::DRAWARRAYS, dimensions) };
    localMeshes.push_back(paddleMesh);
    initializeInput();
}

void ResourceManager::initializeInput() 
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
    auto keyboardRelease{ new GLClickLink[2][128] };

    // GamePad Initialization
    gamePadClick[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_GAMEPAD_BUTTON_Y;
    gamePadClick[GLInput::SINGLEKEYPRESS][0].functionReference = BreakoutActiveFunctions::TOGGLE_FLASHLIGHT;
    gamePadClick[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_GAMEPAD_BUTTON_B;
    gamePadClick[GLInput::SINGLEKEYPRESS][1].functionReference = BreakoutActiveFunctions::RANDOM_FLASHLIGHT_COLOR;
    gamePadClick[GLInput::SINGLEKEYPRESS][2].glfwValue = GLFW_GAMEPAD_BUTTON_START;
    gamePadClick[GLInput::SINGLEKEYPRESS][2].functionReference = BreakoutActiveFunctions::TOGGLE_MENU;

    gamePadStick[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_GAMEPAD_AXIS_LEFT_X;
    gamePadStick[GLInput::SINGLEKEYPRESS][0].functionReference = BreakoutActiveFunctions::MOVE_LEFT_RIGHT_STICK_2D;
    gamePadStick[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_GAMEPAD_AXIS_LEFT_Y;
    gamePadStick[GLInput::SINGLEKEYPRESS][1].functionReference = BreakoutActiveFunctions::MOVE_UP_DOWN_STICK_2D;

    gamePadTrigger[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER;
    gamePadTrigger[GLInput::SINGLEKEYPRESS][0].functionReference = BreakoutActiveFunctions::CAMERA_SPEED_TRIGGER;

    // Keyboard Initialization
    keyboardHold[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_KEY_D;
    keyboardHold[GLInput::SINGLEKEYPRESS][0].functionReference = BreakoutActiveFunctions::MOVE_RIGHT_2D;
    keyboardHold[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_KEY_A;
    keyboardHold[GLInput::SINGLEKEYPRESS][1].functionReference = BreakoutActiveFunctions::MOVE_LEFT_2D;
    keyboardHold[GLInput::SINGLEKEYPRESS][2].glfwValue = GLFW_KEY_UP;
    keyboardHold[GLInput::SINGLEKEYPRESS][2].functionReference = BreakoutActiveFunctions::MOVE_BALL_UP;
    keyboardHold[GLInput::SINGLEKEYPRESS][3].glfwValue = GLFW_KEY_DOWN;
    keyboardHold[GLInput::SINGLEKEYPRESS][3].functionReference = BreakoutActiveFunctions::MOVE_BALL_DOWN;
    keyboardHold[GLInput::SINGLEKEYPRESS][4].glfwValue = GLFW_KEY_LEFT;
    keyboardHold[GLInput::SINGLEKEYPRESS][4].functionReference = BreakoutActiveFunctions::MOVE_BALL_LEFT;
    keyboardHold[GLInput::SINGLEKEYPRESS][5].glfwValue = GLFW_KEY_RIGHT;
    keyboardHold[GLInput::SINGLEKEYPRESS][5].functionReference = BreakoutActiveFunctions::MOVE_BALL_RIGHT;
    keyboardHold[GLInput::SINGLEKEYPRESS][6].glfwValue = GLFW_KEY_EQUAL;
    keyboardHold[GLInput::SINGLEKEYPRESS][6].functionReference = BreakoutActiveFunctions::INCREMENT_BALL_SPEED;
    keyboardHold[GLInput::SINGLEKEYPRESS][7].glfwValue = GLFW_KEY_MINUS;
    keyboardHold[GLInput::SINGLEKEYPRESS][7].functionReference = BreakoutActiveFunctions::DEINCREMENT_BALL_SPEED;


    // Double Key Press Location

    keyboardClick[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_KEY_F;
    keyboardClick[GLInput::SINGLEKEYPRESS][0].functionReference = BreakoutActiveFunctions::TOGGLE_FLASHLIGHT;
    keyboardClick[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_KEY_F1;
    keyboardClick[GLInput::SINGLEKEYPRESS][1].functionReference = BreakoutActiveFunctions::TOGGLE_MENU;
    keyboardClick[GLInput::SINGLEKEYPRESS][2].glfwValue = GLFW_KEY_MINUS;
    keyboardClick[GLInput::SINGLEKEYPRESS][2].functionReference = BreakoutActiveFunctions::CAMERA_DEINCREMENT_SENSITIVITY;
    keyboardClick[GLInput::SINGLEKEYPRESS][3].glfwValue = GLFW_KEY_EQUAL;
    keyboardClick[GLInput::SINGLEKEYPRESS][3].functionReference = BreakoutActiveFunctions::CAMERA_INCREMENT_SENSITIVITY;
    keyboardClick[GLInput::SINGLEKEYPRESS][4].glfwValue = GLFW_KEY_LEFT_BRACKET;
    keyboardClick[GLInput::SINGLEKEYPRESS][4].functionReference = BreakoutActiveFunctions::CAMERA_DEINCREMENT_SPEED;
    keyboardClick[GLInput::SINGLEKEYPRESS][5].glfwValue = GLFW_KEY_RIGHT_BRACKET;
    keyboardClick[GLInput::SINGLEKEYPRESS][5].functionReference = BreakoutActiveFunctions::CAMERA_INCREMENT_SPEED;
    keyboardClick[GLInput::SINGLEKEYPRESS][6].glfwValue = GLFW_KEY_F9;
    keyboardClick[GLInput::SINGLEKEYPRESS][6].functionReference = BreakoutActiveFunctions::TOGGLE_DEVICE_MOUSE_MOVEMENT;
    keyboardClick[GLInput::SINGLEKEYPRESS][7].glfwValue = GLFW_KEY_LEFT_SHIFT;
    keyboardClick[GLInput::SINGLEKEYPRESS][7].functionReference = BreakoutActiveFunctions::INCREASE_PADDLE_SPEED_TOGGLE;
    keyboardClick[GLInput::SINGLEKEYPRESS][8].glfwValue = GLFW_KEY_A;
    keyboardClick[GLInput::SINGLEKEYPRESS][8].functionReference = BreakoutActiveFunctions::MODIFY_PADDLE_DIRECTION_LEFT;
    keyboardClick[GLInput::SINGLEKEYPRESS][9].glfwValue = GLFW_KEY_D;
    keyboardClick[GLInput::SINGLEKEYPRESS][9].functionReference = BreakoutActiveFunctions::MODIFY_PADDLE_DIRECTION_RIGHT;

    keyboardRelease[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_KEY_LEFT_SHIFT;
    keyboardRelease[GLInput::SINGLEKEYPRESS][0].functionReference = BreakoutActiveFunctions::RESET_PADDLE_SPEED;
    keyboardRelease[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_KEY_A;
    keyboardRelease[GLInput::SINGLEKEYPRESS][1].functionReference = BreakoutActiveFunctions::RESET_PADDLE_DIRECTION;
    keyboardRelease[GLInput::SINGLEKEYPRESS][2].glfwValue = GLFW_KEY_D;
    keyboardRelease[GLInput::SINGLEKEYPRESS][2].functionReference = BreakoutActiveFunctions::RESET_PADDLE_DIRECTION;

    // Mouse Scroll Initialization
    mouseScroll[GLInput::SINGLEKEYPRESS][0].glfwValue = 1;
    mouseScroll[GLInput::SINGLEKEYPRESS][0].functionReference = BreakoutActiveFunctions::CAMERA_FOV_MOUSE;



    GLInput* gameplayInput = new GLInput(isGamePadClick, isKeyboardHold, isMouseScroll,
        isGamePadStick, isKeyboardClick, isMouseMovement,
        isGamePadTrigger, gamePadClick, gamePadStick,
        gamePadTrigger, keyboardHold, keyboardClick,
        mouseScroll, mouseMovement, keyboardRelease);
    localInputs.push_back(gameplayInput);

    delete[] gamePadClick;
    delete[] gamePadStick;
    delete[] gamePadTrigger;
    delete[] keyboardHold;
    delete[] keyboardClick;
    delete[] mouseScroll;
    delete[] mouseMovement;
    delete[] keyboardRelease;

    isGamePadClick = false;
    isGamePadStick = false;
    isGamePadTrigger = false;
    isKeyboardHold = true;
    isKeyboardClick = true;
    isMouseScroll = true;
    isMouseMovement = true;

    gamePadClick = new GLClickLink[2][128];
    gamePadStick = new GLJoyStickLink[2][32];
    gamePadTrigger = new GLTriggerLink[2][8];
    keyboardHold = new GLHoldLink[2][128];
    keyboardClick = new GLClickLink[2][128];
    mouseScroll = new GLScrollLink[2][2];
    mouseMovement = new GLMouseMovementLink[2][2];
    keyboardRelease = new GLClickLink[2][128];

    keyboardClick[GLInput::SINGLEKEYPRESS][0].glfwValue = GLFW_KEY_F1;
    keyboardClick[GLInput::SINGLEKEYPRESS][0].functionReference = BreakoutStartFunctions::TOGGLE_MENU;
    keyboardClick[GLInput::SINGLEKEYPRESS][1].glfwValue = GLFW_KEY_F9;
    keyboardClick[GLInput::SINGLEKEYPRESS][1].functionReference = BreakoutStartFunctions::TOGGLE_DEVICE_MOUSE_MOVEMENT;
    keyboardClick[GLInput::SINGLEKEYPRESS][2].glfwValue = GLFW_KEY_SPACE;
    keyboardClick[GLInput::SINGLEKEYPRESS][2].functionReference = BreakoutStartFunctions::TOGGLE_GAME_START;

    GLInput* startMenuInput = new GLInput(isGamePadClick, isKeyboardHold, isMouseScroll,
        isGamePadStick, isKeyboardClick, isMouseMovement,
        isGamePadTrigger, gamePadClick, gamePadStick,
        gamePadTrigger, keyboardHold, keyboardClick,
        mouseScroll, mouseMovement, keyboardRelease);
    localInputs.push_back(startMenuInput);

    delete[] gamePadClick;
    delete[] gamePadStick;
    delete[] gamePadTrigger;
    delete[] keyboardHold;
    delete[] keyboardClick;
    delete[] mouseScroll;
    delete[] mouseMovement;
    delete[] keyboardRelease;
}