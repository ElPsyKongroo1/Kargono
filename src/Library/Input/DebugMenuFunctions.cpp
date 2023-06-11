#include "Input.h"
#include "../Library.h"
#include "InputFunctions.h"


namespace DebugMenuFunctions 
{
    bool TOGGLE_MENU(GLInputLink* gamePadButton)
    {
        if (Resources::currentApplication->currentInput != &Resources::inputManager.debugMenuInput)
        {
            glfwSetInputMode(Resources::currentApplication->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            updateType = true;
            typeChange[0] = Resources::currentApplication->currentInput;
            typeChange[1] = &Resources::inputManager.debugMenuInput;
            if (gamePadButton != nullptr) oldButton = gamePadButton;
            Resources::currentApplication->currentCamera->firstMouse = true;
            Resources::windowManager.mainMenu.isRendering = true;
            Resources::currentApplication->currentWindow = &Resources::windowManager.mainMenu;
        }
        else if (Resources::currentApplication->currentInput == &Resources::inputManager.debugMenuInput)
        {
            glfwSetInputMode(Resources::currentApplication->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            updateType = true;
            typeChange[0] = Resources::currentApplication->currentInput;
            typeChange[1] = Resources::currentApplication->defaultInput;
            Resources::windowManager.mainMenu.closeChildren();
            Resources::currentApplication->currentWindow = nullptr;
        }
        return true;
    }

    bool CLOSE_CURRENT_WINDOW(GLInputLink* gamePadButton)
    {
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
        return false;
    }
}