#include "../Input.h"
#include "../../../Rendering/Library.h"
#include "../MiscFunctions.h"


namespace DebugMenuFunctions 
{
    bool TOGGLE_MENU(GLInputLink* gamePadButton)
    {
        if (Resources::currentRenderer->currentInput != &Resources::inputManager.debugMenuInput)
        {
            glfwSetInputMode(Resources::currentRenderer->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            updateType = true;
            typeChange[0] = Resources::currentRenderer->currentInput;
            typeChange[1] = &Resources::inputManager.debugMenuInput;
            if (gamePadButton != nullptr) oldButton = gamePadButton;
            Resources::currentRenderer->currentCamera->firstMouse = true;
            Resources::windowManager.mainMenu.isRendering = true;
            Resources::currentRenderer->currentWindow = &Resources::windowManager.mainMenu;
        }
        else if (Resources::currentRenderer->currentInput == &Resources::inputManager.debugMenuInput)
        {
            glfwSetInputMode(Resources::currentRenderer->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            updateType = true;
            typeChange[0] = Resources::currentRenderer->currentInput;
            typeChange[1] = Resources::currentRenderer->defaultInput;
            Resources::windowManager.mainMenu.closeChildren();
            Resources::currentRenderer->currentWindow = nullptr;
        }
        return true;
    }

    bool CLOSE_CURRENT_WINDOW(GLInputLink* gamePadButton)
    {
        Resources::currentRenderer->currentWindow->isRendering = false;
        Resources::currentRenderer->currentWindow = Resources::currentRenderer->currentWindow->parent;
        if (Resources::currentRenderer->currentWindow == nullptr)
        {
            glfwSetInputMode(Resources::currentRenderer->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            updateType = true;
            typeChange[0] = Resources::currentRenderer->currentInput;
            typeChange[1] = Resources::currentRenderer->defaultInput;

            Resources::windowManager.mainMenu.closeChildren();
            Resources::currentRenderer->currentWindow = nullptr;
        }
        return false;
    }

    bool EXIT_APPLICATION()
    {
        glfwSetWindowShouldClose(Resources::currentRenderer->window, true);
        return false;
    }
}