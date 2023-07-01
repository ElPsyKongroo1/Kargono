#include "../../../../Library/Application/Input/Input.h"
#include "../../../../Library/Library.h"
#include "../../../../Library/Application/Input/MiscFunctions.h"
#include "BreakoutActiveFunctions.h"
#include "../BreakoutObject/BreakoutObject.h"


namespace BreakoutActiveFunctions
{
    // Mouse Scroll Wheel
    void CAMERA_FOV_MOUSE(double xoffset, double yoffset)
    {
        Resources::currentApplication->renderer->currentCamera->fov -= (float)yoffset;
        if (Resources::currentApplication->renderer->currentCamera->fov < 1.0f)
            Resources::currentApplication->renderer->currentCamera->fov = 1.0f;
        if (Resources::currentApplication->renderer->currentCamera->fov > 45.0f)
            Resources::currentApplication->renderer->currentCamera->fov = 45.0f;
    }
    // Button/Keyboard Click
    bool TOGGLE_FLASHLIGHT(GLInputLink* gamePadButton)
    {
        bool currentFlash;
        bool decision;
        bool foundFlashLight;

        currentFlash = Resources::currentApplication->renderer->currentCamera->isFlashLight;
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
            Resources::currentApplication->renderer->currentCamera->isFlashLight = true;
            for (int i = 0; i < Resources::currentApplication->renderer->lightSourceRenderBuffer.size(); i++)
            {
                if (&Resources::currentApplication->renderer->currentCamera->flashLight == Resources::currentApplication->renderer->lightSourceRenderBuffer.at(i))
                {
                    foundFlashLight = true;
                    break;
                }
            }
            if (!foundFlashLight)
            {
                Resources::currentApplication->renderer->lightSourceRenderBuffer.push_back(&Resources::currentApplication->renderer->currentCamera->flashLight);
            }
        }
        else
        {
            Resources::currentApplication->renderer->currentCamera->isFlashLight = false;
            for (int i = 0; i < Resources::currentApplication->renderer->lightSourceRenderBuffer.size(); i++)
            {
                if (&Resources::currentApplication->renderer->currentCamera->flashLight == Resources::currentApplication->renderer->lightSourceRenderBuffer.at(i))
                {
                    foundFlashLight = true;
                    Resources::currentApplication->renderer->lightSourceRenderBuffer.erase(Resources::currentApplication->renderer->lightSourceRenderBuffer.begin() + i);
                    break;
                }
            }
        }
        return false;
    }


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
        Resources::currentApplication->renderer->currentCamera->flashLight.ambientColor = randomColor * 0.1f;
        Resources::currentApplication->renderer->currentCamera->flashLight.diffuseColor = randomColor;
        Resources::currentApplication->renderer->currentCamera->flashLight.specularColor = randomColor;
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

    bool CAMERA_DEINCREMENT_SENSITIVITY(GLInputLink* gamePadButton)
    {
        if ((Resources::currentApplication->renderer->currentCamera->currentPanningSpeed - Resources::currentApplication->renderer->currentCamera->defaultPanningSpeed) < Resources::currentApplication->renderer->currentCamera->defaultPanningSpeed)
            return false;
        Resources::currentApplication->renderer->currentCamera->currentPanningSpeed -= Resources::currentApplication->renderer->currentCamera->defaultPanningSpeed;
        return false;
    }

    bool CAMERA_INCREMENT_SENSITIVITY(GLInputLink* gamePadButton)
    {
        if ((Resources::currentApplication->renderer->currentCamera->currentPanningSpeed + Resources::currentApplication->renderer->currentCamera->defaultPanningSpeed) > (Resources::currentApplication->renderer->currentCamera->defaultPanningSpeed * 10))
            return false;
        Resources::currentApplication->renderer->currentCamera->currentPanningSpeed += Resources::currentApplication->renderer->currentCamera->defaultPanningSpeed;
        return false;
    }
    bool CAMERA_DEINCREMENT_SPEED(GLInputLink* gamePadButton)
    {
        if ((Resources::currentApplication->renderer->currentCamera->currentMovementSpeed - Resources::currentApplication->renderer->currentCamera->defaultMovementSpeed) < Resources::currentApplication->renderer->currentCamera->defaultMovementSpeed)
            return false;
        Resources::currentApplication->renderer->currentCamera->setCurrentCameraSpeed(Resources::currentApplication->renderer->currentCamera->currentMovementSpeed - Resources::currentApplication->renderer->currentCamera->defaultMovementSpeed);
        return false;
    }
    bool CAMERA_INCREMENT_SPEED(GLInputLink* gamePadButton)
    {
        if ((Resources::currentApplication->renderer->currentCamera->currentMovementSpeed + Resources::currentApplication->renderer->currentCamera->defaultMovementSpeed) > (Resources::currentApplication->renderer->currentCamera->defaultMovementSpeed * 50))
            return false;
        Resources::currentApplication->renderer->currentCamera->setCurrentCameraSpeed(Resources::currentApplication->renderer->currentCamera->currentMovementSpeed + Resources::currentApplication->renderer->currentCamera->defaultMovementSpeed);
        return false;
    }
    bool INCREASE_PADDLE_SPEED_TOGGLE(GLInputLink* gamePadButton) 
    {
        auto paddle = Resources::currentGame->resourceManager->currentLevel->paddle;
        paddle->currentSpeed = 1.7f * paddle->baseSpeed;
        return false;
    }
    bool MODIFY_PADDLE_DIRECTION_LEFT(GLInputLink* gamePadButton) 
    {
        auto paddle = Resources::currentGame->resourceManager->currentLevel->paddle;
        paddle->direction = glm::vec3(-0.8f, 0.2f, 0.0f);
        return false;
    }
    bool MODIFY_PADDLE_DIRECTION_RIGHT(GLInputLink* gamePadButton)
    {
        auto paddle = Resources::currentGame->resourceManager->currentLevel->paddle;
        paddle->direction = glm::vec3(0.8f, 0.2f, 0.0f);
        return false;
    }
    


    bool RESET_PADDLE_DIRECTION(GLInputLink* gamePadButton) 
    {
        auto paddle = Resources::currentGame->resourceManager->currentLevel->paddle;
        paddle->direction = glm::vec3(0.0f, 0.0f, 0.0f);
        return false;
    }
    bool RESET_PADDLE_SPEED(GLInputLink* gamePadButton)
    {
        auto paddle = Resources::currentGame->resourceManager->currentLevel->paddle;
        paddle->currentSpeed = paddle->baseSpeed;
        return false;
    }



    bool INCREMENT_BALL_SPEED()
    {
        auto ball = Resources::currentGame->resourceManager->currentLevel->ball;
        ball->currentSpeed += 300.0f * Resources::deltaTime;
        return false;
    }
    bool DEINCREMENT_BALL_SPEED()
    {
        auto ball = Resources::currentGame->resourceManager->currentLevel->ball;
        ball->currentSpeed -= 300.0f * Resources::deltaTime;
        if (ball->currentSpeed < 0.0f) { ball->currentSpeed = 0; }
        return false;
    }
    bool MOVE_BALL_RIGHT() 
    {
        auto ball = Resources::currentGame->resourceManager->currentLevel->ball;
        ball->orientation.translation.x += ball->currentSpeed * Resources::deltaTime;
        return false;
    }
    bool MOVE_BALL_LEFT()
    {
        auto ball = Resources::currentGame->resourceManager->currentLevel->ball;
        ball->orientation.translation.x -= ball->currentSpeed * Resources::deltaTime;
        return false;
    }
    bool MOVE_BALL_UP()
    {
        auto ball = Resources::currentGame->resourceManager->currentLevel->ball;
        ball->orientation.translation.y += ball->currentSpeed * Resources::deltaTime;
        return false;
    }
    bool MOVE_BALL_DOWN()
    {
        auto ball = Resources::currentGame->resourceManager->currentLevel->ball;
        ball->orientation.translation.y -= ball->currentSpeed * Resources::deltaTime;
        return false;
    }

    bool MOVE_RIGHT_2D()
    {
        auto paddle = Resources::currentGame->resourceManager->currentLevel->paddle;
        float maxBorder = Resources::currentGame->renderer->currentCamera->frustrumDimensions.widthDimension.y - (static_cast<ShapeRenderer*>(paddle->renderer)->mesh->dimensions.x * paddle->orientation.scale.x);

        if (paddle->orientation.translation.x >= maxBorder)
        {
            paddle->orientation.translation.x = maxBorder;
        }
        else 
        {
            paddle->orientation.translation.x += paddle->currentSpeed * Resources::deltaTime;
        }
       
        return false;
    }
    bool MOVE_LEFT_2D()
    {
        auto paddle = Resources::currentGame->resourceManager->currentLevel->paddle;
        float minBorder = -(Resources::currentGame->renderer->currentCamera->frustrumDimensions.widthDimension.y - (static_cast<ShapeRenderer*>(paddle->renderer)->mesh->dimensions.x * paddle->orientation.scale.x));

        if (paddle->orientation.translation.x <= minBorder)
        {
            paddle->orientation.translation.x = minBorder;
        }
        else 
        {
            paddle->orientation.translation.x -= paddle->currentSpeed * Resources::deltaTime;
        }
        
        return false;
    }

    bool MOVE_LEFT_RIGHT_STICK_2D(float axis)
    {
        float cameraSpeed = Resources::currentApplication->renderer->currentCamera->currentMovementSpeed * Resources::deltaTime;
        cameraSpeed = Resources::currentApplication->renderer->currentCamera->currentMovementSpeed * Resources::deltaTime;
        Resources::currentApplication->renderer->currentCamera->orientation.cameraPosition += glm::vec3(cameraSpeed * axis, 0.0f, 0.0f);
        return false;
    }
    bool MOVE_UP_DOWN_STICK_2D(float axis)
    {
        float cameraSpeed = Resources::currentApplication->renderer->currentCamera->currentMovementSpeed * Resources::deltaTime;
        cameraSpeed = Resources::currentApplication->renderer->currentCamera->currentMovementSpeed * Resources::deltaTime;
        Resources::currentApplication->renderer->currentCamera->orientation.cameraPosition -= glm::vec3(0.0f, cameraSpeed * axis, 0.0f);
        return false;
    }

    void CAMERA_SPEED_TRIGGER(float axis)
    {
        if (axis < -0.15)
        {
            Resources::currentApplication->renderer->currentCamera->setCurrentCameraSpeed(Resources::currentApplication->renderer->currentCamera->defaultMovementSpeed);
        }
        if (axis < -0.5f)
        {
            Resources::currentApplication->renderer->currentCamera->setCurrentCameraSpeed(Resources::currentApplication->renderer->currentCamera->defaultMovementSpeed * 3.0f);
        }
        else if (axis < 0.0f)
        {
            Resources::currentApplication->renderer->currentCamera->setCurrentCameraSpeed(Resources::currentApplication->renderer->currentCamera->defaultMovementSpeed * 5.0f);
        }
        else if (axis < 0.5f)
        {
            Resources::currentApplication->renderer->currentCamera->setCurrentCameraSpeed(Resources::currentApplication->renderer->currentCamera->defaultMovementSpeed * 7.0f);
        }
        else if (axis < 0.75f)
        {
            Resources::currentApplication->renderer->currentCamera->setCurrentCameraSpeed(Resources::currentApplication->renderer->currentCamera->defaultMovementSpeed * 8.0f);
        }
        else if (axis < 0.875f)
        {
            Resources::currentApplication->renderer->currentCamera->setCurrentCameraSpeed(Resources::currentApplication->renderer->currentCamera->defaultMovementSpeed * 9.0f);
        }
        else
        {
            Resources::currentApplication->renderer->currentCamera->setCurrentCameraSpeed(Resources::currentApplication->renderer->currentCamera->defaultMovementSpeed * 10.0f);
        }
    }


}