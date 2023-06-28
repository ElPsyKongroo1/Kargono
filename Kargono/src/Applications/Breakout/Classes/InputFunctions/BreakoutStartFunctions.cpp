#include "../../../Library/Application/Input/Input.h"
#include "../../../Library/Library.h"
#include "../../../Library/Application/Input/MiscFunctions.h"
#include "BreakoutStartFunctions.h"
#include "../BreakoutObject/BreakoutObject.h"


namespace BreakoutStartFunctions
{
    // Mouse Scroll Wheel
   
    // Button/Keyboard Click


    bool TOGGLE_DEVICE_MOUSE_MOVEMENT(GLInputLink* gamePadButton)
    {
        if (Resources::currentApplication->currentInput->isMouseMovement) { Resources::currentApplication->currentInput->isMouseMovement = false; }
        else { Resources::currentApplication->currentInput->isMouseMovement = true; Resources::currentApplication->renderer->currentCamera->firstMouse = true; }
        return false;
    }

    bool EXIT_APPLICATION(GLInputLink* gamePadButton)
    {
        //glfwSetWindowMonitor(Resources::currentApplication->window, NULL, 0, 0, Resources::currentApplication->screenDimension.x, Resources::currentApplication->screenDimension.y, NULL);
        glfwSetWindowShouldClose(Resources::currentApplication->renderer->window, true);
        return false;
    }

    bool TOGGLE_MENU(GLInputLink* gamePadButton)
    {
        if (Resources::currentApplication->currentInput != Resources::inputManager.debugMenuInput)
        {
            glfwSetInputMode(Resources::currentApplication->renderer->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            updateType = true;
            typeChange[0] = Resources::currentApplication->currentInput;
            typeChange[1] = Resources::inputManager.debugMenuInput;
            if (gamePadButton != nullptr) oldButton = gamePadButton;
            Resources::currentApplication->renderer->currentCamera->firstMouse = true;
            Resources::windowManager.mainMenu.isRendering = true;
            Resources::currentApplication->renderer->currentWindow = &Resources::windowManager.mainMenu;
        }
        else if (Resources::currentApplication->currentInput == Resources::inputManager.debugMenuInput)
        {
            glfwSetInputMode(Resources::currentApplication->renderer->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            updateType = true;
            typeChange[0] = Resources::currentApplication->currentInput;
            typeChange[1] = Resources::currentApplication->recentInput;
            Resources::windowManager.mainMenu.closeChildren();
            Resources::currentApplication->renderer->currentWindow = nullptr;
        }
        return true;
    }

    bool TOGGLE_GAME_START(GLInputLink* gamePadButton)
    {
        if (Resources::currentApplication->currentInput != Resources::currentGame->resourceManager->localInputs.at(1))
        {
            updateType = true;
            typeChange[0] = Resources::currentApplication->currentInput;
            typeChange[1] = Resources::currentGame->resourceManager->localInputs.at(1);
            if (gamePadButton != nullptr) oldButton = gamePadButton;
            Resources::currentApplication->renderer->currentCamera->firstMouse = true;
            Resources::currentGame->State = GameApplication::GAME_MENU;
        }
        else if (Resources::currentApplication->currentInput == Resources::currentGame->resourceManager->localInputs.at(1))
        {
            updateType = true;
            typeChange[0] = Resources::currentApplication->currentInput;
            typeChange[1] = Resources::currentGame->resourceManager->localInputs.at(0);
            Resources::currentGame->State = GameApplication::GAME_ACTIVE;
        }
        return true;
    }

}

