#include "Input.h"
#include "../Library.h"
#include "InputFunctions.h"

namespace Default2DFunctions 
{
    // Mouse Scroll Wheel
    void CAMERA_FOV_MOUSE(double xoffset, double yoffset)
    {
        Resources::currentApplication->currentCamera->fov -= (float)yoffset;
        if (Resources::currentApplication->currentCamera->fov < 1.0f)
            Resources::currentApplication->currentCamera->fov = 1.0f;
        if (Resources::currentApplication->currentCamera->fov > 45.0f)
            Resources::currentApplication->currentCamera->fov = 45.0f;
    }
    // Button/Keyboard Click
    bool TOGGLE_FLASHLIGHT(GLInputLink* gamePadButton)
    {
        bool currentFlash;
        bool decision;
        bool foundFlashLight;

        currentFlash = Resources::currentApplication->currentCamera->isFlashLight;
        if (currentFlash)
        {
            decision = false;
        }
        else
        {
            decision = true;
        }
        foundFlashLight = false;
        if (decision)
        {
            Resources::currentApplication->currentCamera->isFlashLight = true;
            for (int i = 0; i < Resources::currentApplication->lightSourceRenderBuffer.size(); i++)
            {
                if (&Resources::currentApplication->currentCamera->flashLight == Resources::currentApplication->lightSourceRenderBuffer.at(i))
                {
                    foundFlashLight = true;
                    break;
                }
            }
            if (!foundFlashLight)
            {
                Resources::currentApplication->lightSourceRenderBuffer.push_back(&Resources::currentApplication->currentCamera->flashLight);
            }
        }
        else
        {
            Resources::currentApplication->currentCamera->isFlashLight = false;
            for (int i = 0; i < Resources::currentApplication->lightSourceRenderBuffer.size(); i++)
            {
                if (&Resources::currentApplication->currentCamera->flashLight == Resources::currentApplication->lightSourceRenderBuffer.at(i))
                {
                    foundFlashLight = true;
                    Resources::currentApplication->lightSourceRenderBuffer.erase(Resources::currentApplication->lightSourceRenderBuffer.begin() + i);
                    break;
                }
            }
        }
        return false;
    }


    bool TOGGLE_DEVICE_MOUSE_MOVEMENT(GLInputLink* gamePadButton)
    {
        if (Resources::currentApplication->currentInput->isMouseMovement) { Resources::currentApplication->currentInput->isMouseMovement = false; }
        else { Resources::currentApplication->currentInput->isMouseMovement = true; Resources::currentApplication->currentCamera->firstMouse = true; }
        return false;
    }

    bool EXIT_APPLICATION(GLInputLink* gamePadButton) 
    {
        //glfwSetWindowMonitor(Resources::currentApplication->window, NULL, 0, 0, Resources::currentApplication->screenDimension.x, Resources::currentApplication->screenDimension.y, NULL);
        glfwSetWindowShouldClose(Resources::currentApplication->window, true);
        return false;
    }

    bool RANDOM_FLASHLIGHT_COLOR(GLInputLink* gamePadButton) 
    {
        glm::vec3 randomColor;
        float r, g, b;
    
        r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        if (r < 0.4f)
        {
            r += 0.4f;
        }
        if (r < 0.7f)
        {
            r += 0.2f;
        }
        g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        if (g < 0.4f)
        {
            g += 0.4f;
        }
        if (g < 0.7f)
        {
            g += 0.2f;
        }
        b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        if (b < 0.4f)
        {
            b += 0.4f;
        }
        if (b < 0.7f)
        {
            b += 0.2f;
        }
        randomColor = glm::vec3(r, g, b);
        Resources::currentApplication->currentCamera->flashLight.ambientColor = randomColor * 0.1f;
        Resources::currentApplication->currentCamera->flashLight.diffuseColor = randomColor;
        Resources::currentApplication->currentCamera->flashLight.specularColor = randomColor;
        return false;
    }

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

    bool CAMERA_DEINCREMENT_SENSITIVITY (GLInputLink* gamePadButton)
    {
        if ((Resources::currentApplication->currentCamera->currentPanningSpeed - Resources::currentApplication->currentCamera->defaultPanningSpeed) < Resources::currentApplication->currentCamera->defaultPanningSpeed)
            return false;
        Resources::currentApplication->currentCamera->currentPanningSpeed -= Resources::currentApplication->currentCamera->defaultPanningSpeed;
        return false;
    }

    bool CAMERA_INCREMENT_SENSITIVITY (GLInputLink* gamePadButton)
    {
        if ((Resources::currentApplication->currentCamera->currentPanningSpeed + Resources::currentApplication->currentCamera->defaultPanningSpeed) > (Resources::currentApplication->currentCamera->defaultPanningSpeed * 10))
            return false;
        Resources::currentApplication->currentCamera->currentPanningSpeed += Resources::currentApplication->currentCamera->defaultPanningSpeed;
        return false;
    }
    bool CAMERA_DEINCREMENT_SPEED (GLInputLink* gamePadButton)
    {
        if ((Resources::currentApplication->currentCamera->currentMovementSpeed - Resources::currentApplication->currentCamera->defaultMovementSpeed) < Resources::currentApplication->currentCamera->defaultMovementSpeed)
            return false;
        Resources::currentApplication->currentCamera->setCurrentCameraSpeed(Resources::currentApplication->currentCamera->currentMovementSpeed - Resources::currentApplication->currentCamera->defaultMovementSpeed);
        return false;
    }
    bool CAMERA_INCREMENT_SPEED (GLInputLink* gamePadButton)
    {
        if ((Resources::currentApplication->currentCamera->currentMovementSpeed + Resources::currentApplication->currentCamera->defaultMovementSpeed) > (Resources::currentApplication->currentCamera->defaultMovementSpeed * 50))
            return false;
        Resources::currentApplication->currentCamera->setCurrentCameraSpeed(Resources::currentApplication->currentCamera->currentMovementSpeed + Resources::currentApplication->currentCamera->defaultMovementSpeed);
        return false;
    }
}