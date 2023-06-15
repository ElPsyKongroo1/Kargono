#include "../Input.h"
#include "../../../Library.h"
#include "../MiscFunctions.h"


namespace DebugMenuFunctions 
{
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
            typeChange[1] = Resources::currentApplication->defaultInput;
            Resources::windowManager.mainMenu.closeChildren();
            Resources::currentApplication->renderer->currentWindow = nullptr;
        }
        return true;
    }

    bool CLOSE_CURRENT_WINDOW(GLInputLink* gamePadButton)
    {
        Resources::currentApplication->renderer->currentWindow->isRendering = false;
        Resources::currentApplication->renderer->currentWindow = Resources::currentApplication->renderer->currentWindow->parent;
        if (Resources::currentApplication->renderer->currentWindow == nullptr)
        {
            glfwSetInputMode(Resources::currentApplication->renderer->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            updateType = true;
            typeChange[0] = Resources::currentApplication->currentInput;
            typeChange[1] = Resources::currentApplication->defaultInput;

            Resources::windowManager.mainMenu.closeChildren();
            Resources::currentApplication->renderer->currentWindow = nullptr;
        }
        return false;
    }

    bool EXIT_APPLICATION()
    {
        glfwSetWindowShouldClose(Resources::currentApplication->renderer->window, true);
        return false;
    }
}