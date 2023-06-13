#include "../Input.h"
#include "../../../Rendering/Library.h"
#include "../MiscFunctions.h"

namespace Default3DFunctions {
    // Mouse Movement
    void CAMERA_YAW_PITCH_MOUSE(double xpos, double ypos)
    {
        float xoffset;
        float yoffset;
        glm::vec3 direction;

        if (Resources::currentRenderer->currentCamera->firstMouse) // initially set to true
        {
            Resources::currentRenderer->currentCamera->lastX = xpos;
            Resources::currentRenderer->currentCamera->lastY = ypos;
            Resources::currentRenderer->currentCamera->firstMouse = false;
        }

        xoffset = xpos - Resources::currentRenderer->currentCamera->lastX;
        yoffset = Resources::currentRenderer->currentCamera->lastY - ypos; // reversed since y-coordinates range from bottom to top
        Resources::currentRenderer->currentCamera->lastX = xpos;
        Resources::currentRenderer->currentCamera->lastY = ypos;

        xoffset *= Resources::currentRenderer->currentCamera->currentPanningSpeed;
        yoffset *= Resources::currentRenderer->currentCamera->currentPanningSpeed;

        Resources::currentRenderer->currentCamera->eulerAngle.yaw += xoffset;
        Resources::currentRenderer->currentCamera->eulerAngle.pitch += yoffset;

        if (Resources::currentRenderer->currentCamera->eulerAngle.pitch > 89.0f)
            Resources::currentRenderer->currentCamera->eulerAngle.pitch = 89.0f;
        if (Resources::currentRenderer->currentCamera->eulerAngle.pitch < -89.0f)
            Resources::currentRenderer->currentCamera->eulerAngle.pitch = -89.0f;


        direction.x = cos(glm::radians(Resources::currentRenderer->currentCamera->eulerAngle.yaw)) * cos(glm::radians(Resources::currentRenderer->currentCamera->eulerAngle.pitch));
        direction.y = sin(glm::radians(Resources::currentRenderer->currentCamera->eulerAngle.pitch));
        direction.z = sin(glm::radians(Resources::currentRenderer->currentCamera->eulerAngle.yaw)) * cos(glm::radians(Resources::currentRenderer->currentCamera->eulerAngle.pitch));
        Resources::currentRenderer->currentCamera->orientation.cameraFront = glm::normalize(direction);
    }
    // Mouse Scroll Wheel
    void CAMERA_FOV_MOUSE(double xoffset, double yoffset) 
    {
        Resources::currentRenderer->currentCamera->fov -= (float)yoffset;
        if (Resources::currentRenderer->currentCamera->fov < 1.0f)
            Resources::currentRenderer->currentCamera->fov = 1.0f;
        if (Resources::currentRenderer->currentCamera->fov > 45.0f)
            Resources::currentRenderer->currentCamera->fov = 45.0f;
    }
    // Button/Keyboard Click
    bool TOGGLE_FLASHLIGHT(GLInputLink* gamePadButton)
    {
        bool currentFlash;
        bool decision;
        bool foundFlashLight;

        currentFlash = Resources::currentRenderer->currentCamera->isFlashLight;
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
            Resources::currentRenderer->currentCamera->isFlashLight = true;
            for (int i = 0; i < Resources::currentRenderer->lightSourceRenderBuffer.size(); i++)
            {
                if (&Resources::currentRenderer->currentCamera->flashLight == Resources::currentRenderer->lightSourceRenderBuffer.at(i))
                {
                    foundFlashLight = true;
                    break;
                }
            }
            if (!foundFlashLight)
            {
                Resources::currentRenderer->lightSourceRenderBuffer.push_back(&Resources::currentRenderer->currentCamera->flashLight);
            }
        }
        else
        {
            Resources::currentRenderer->currentCamera->isFlashLight = false;
            for (int i = 0; i < Resources::currentRenderer->lightSourceRenderBuffer.size(); i++)
            {
                if (&Resources::currentRenderer->currentCamera->flashLight == Resources::currentRenderer->lightSourceRenderBuffer.at(i))
                {
                    foundFlashLight = true;
                    Resources::currentRenderer->lightSourceRenderBuffer.erase(Resources::currentRenderer->lightSourceRenderBuffer.begin() + i);
                    break;
                }
            }
        }
        return false;
    }


    bool TOGGLE_DEVICE_MOUSE_MOVEMENT(GLInputLink* gamePadButton)
    {
        if (Resources::currentRenderer->currentInput->isMouseMovement) { Resources::currentRenderer->currentInput->isMouseMovement = false; }
        else { Resources::currentRenderer->currentInput->isMouseMovement = true; Resources::currentRenderer->currentCamera->firstMouse = true; }
        return false;
    }

    bool EXIT_APPLICATION(GLInputLink* gamePadButton)
    {
        //glfwSetWindowMonitor(Resources::currentApplication->window, NULL, 0, 0, Resources::currentApplication->screenDimension.x, Resources::currentApplication->screenDimension.y, NULL);
        glfwSetWindowShouldClose(Resources::currentRenderer->window, true);
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
        Resources::currentRenderer->currentCamera->flashLight.ambientColor = randomColor * 0.1f;
        Resources::currentRenderer->currentCamera->flashLight.diffuseColor = randomColor;
        Resources::currentRenderer->currentCamera->flashLight.specularColor = randomColor;
        return false;
    }

    bool TOGGLE_MENU(GLInputLink* gamePadButton)
    {
        if (Resources::currentRenderer->currentInput != Resources::inputManager.debugMenuInput)
        {
            glfwSetInputMode(Resources::currentRenderer->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            updateType = true;
            typeChange[0] = Resources::currentRenderer->currentInput;
            typeChange[1] = Resources::inputManager.debugMenuInput;
            if (gamePadButton != nullptr) oldButton = gamePadButton;
            Resources::currentRenderer->currentCamera->firstMouse = true;
            Resources::windowManager.mainMenu.isRendering = true;
            Resources::currentRenderer->currentWindow = &Resources::windowManager.mainMenu;
        }
        else if (Resources::currentRenderer->currentInput == Resources::inputManager.debugMenuInput)
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

    bool CAMERA_DEINCREMENT_SENSITIVITY(GLInputLink* gamePadButton)
    {
        if ((Resources::currentRenderer->currentCamera->currentPanningSpeed - Resources::currentRenderer->currentCamera->defaultPanningSpeed) < Resources::currentRenderer->currentCamera->defaultPanningSpeed)
            return false;
        Resources::currentRenderer->currentCamera->currentPanningSpeed -= Resources::currentRenderer->currentCamera->defaultPanningSpeed;
        return false;
    }

    bool CAMERA_INCREMENT_SENSITIVITY(GLInputLink* gamePadButton)
    {
        if ((Resources::currentRenderer->currentCamera->currentPanningSpeed + Resources::currentRenderer->currentCamera->defaultPanningSpeed) > (Resources::currentRenderer->currentCamera->defaultPanningSpeed * 10))
            return false;
        Resources::currentRenderer->currentCamera->currentPanningSpeed += Resources::currentRenderer->currentCamera->defaultPanningSpeed;
        return false;
    }
    bool CAMERA_DEINCREMENT_SPEED(GLInputLink* gamePadButton)
    {
        if ((Resources::currentRenderer->currentCamera->currentMovementSpeed - Resources::currentRenderer->currentCamera->defaultMovementSpeed) < Resources::currentRenderer->currentCamera->defaultMovementSpeed)
            return false;
        Resources::currentRenderer->currentCamera->setCurrentCameraSpeed(Resources::currentRenderer->currentCamera->currentMovementSpeed - Resources::currentRenderer->currentCamera->defaultMovementSpeed);
        return false;

    }
    bool CAMERA_INCREMENT_SPEED(GLInputLink* gamePadButton)
    {
        if ((Resources::currentRenderer->currentCamera->currentMovementSpeed + Resources::currentRenderer->currentCamera->defaultMovementSpeed) > (Resources::currentRenderer->currentCamera->defaultMovementSpeed * 50))
            return false;
        Resources::currentRenderer->currentCamera->setCurrentCameraSpeed(Resources::currentRenderer->currentCamera->currentMovementSpeed + Resources::currentRenderer->currentCamera->defaultMovementSpeed);
        return false;
    }

    bool MOVE_FORWARD_KEY()
    {
        float cameraSpeed;
        cameraSpeed = Resources::currentRenderer->currentCamera->currentMovementSpeed * Resources::deltaTime;
        Resources::currentRenderer->currentCamera->orientation.cameraPosition += cameraSpeed * Resources::currentRenderer->currentCamera->orientation.cameraFront;
        return false;
    }

    bool MOVE_BACKWARD_KEY()
    {
        float cameraSpeed;
        cameraSpeed = Resources::currentRenderer->currentCamera->currentMovementSpeed * Resources::deltaTime;
        Resources::currentRenderer->currentCamera->orientation.cameraPosition -= cameraSpeed * Resources::currentRenderer->currentCamera->orientation.cameraFront;
        return false;
    }

    bool MOVE_LEFT_KEY () 
    {
        float cameraSpeed;
        cameraSpeed = Resources::currentRenderer->currentCamera->currentMovementSpeed * Resources::deltaTime;
        Resources::currentRenderer->currentCamera->orientation.cameraPosition -= glm::normalize(glm::cross(Resources::currentRenderer->currentCamera->orientation.cameraFront, Resources::currentRenderer->currentCamera->orientation.cameraUp)) * cameraSpeed;
        return false;
    }

    bool MOVE_RIGHT_KEY () 
    {
        float cameraSpeed;
        cameraSpeed = Resources::currentRenderer->currentCamera->currentMovementSpeed * Resources::deltaTime;
        Resources::currentRenderer->currentCamera->orientation.cameraPosition += glm::normalize(glm::cross(Resources::currentRenderer->currentCamera->orientation.cameraFront, Resources::currentRenderer->currentCamera->orientation.cameraUp)) * cameraSpeed;
        return false;
    }

    bool CAMERA_PITCH_UP_KEY () 
    {
        Resources::currentRenderer->currentCamera->eulerAngle.pitch += Resources::currentRenderer->currentCamera->currentPanningSpeed;
        if (Resources::currentRenderer->currentCamera->eulerAngle.pitch > 89.0f)
            Resources::currentRenderer->currentCamera->eulerAngle.pitch = 89.0f;
        if (Resources::currentRenderer->currentCamera->eulerAngle.pitch < -89.0f)
            Resources::currentRenderer->currentCamera->eulerAngle.pitch = -89.0f;
        return true;
    }
        

    bool CAMERA_PITCH_DOWN_KEY () 
    {
        Resources::currentRenderer->currentCamera->eulerAngle.pitch -= Resources::currentRenderer->currentCamera->currentPanningSpeed;
        if (Resources::currentRenderer->currentCamera->eulerAngle.pitch > 89.0f)
            Resources::currentRenderer->currentCamera->eulerAngle.pitch = 89.0f;
        if (Resources::currentRenderer->currentCamera->eulerAngle.pitch < -89.0f)
            Resources::currentRenderer->currentCamera->eulerAngle.pitch = -89.0f;
        return true;
    }
        

    bool CAMERA_YAW_LEFT_KEY () 
    {
        Resources::currentRenderer->currentCamera->eulerAngle.yaw -= Resources::currentRenderer->currentCamera->currentPanningSpeed;
        return true;
    }
        

    bool CAMERA_YAW_RIGHT_KEY () 
    {
        Resources::currentRenderer->currentCamera->eulerAngle.yaw += Resources::currentRenderer->currentCamera->currentPanningSpeed;
        return true;
    }
   
    bool MOVE_LEFT_RIGHT_STICK (float axis)
    {
        float cameraSpeed = Resources::currentRenderer->currentCamera->currentMovementSpeed * Resources::deltaTime;
        cameraSpeed = Resources::currentRenderer->currentCamera->currentMovementSpeed * Resources::deltaTime;
        Resources::currentRenderer->currentCamera->orientation.cameraPosition += glm::normalize(glm::cross(Resources::currentRenderer->currentCamera->orientation.cameraFront, Resources::currentRenderer->currentCamera->orientation.cameraUp)) * (cameraSpeed * axis);
        return false;
    }
    bool MOVE_UP_DOWN_STICK (float axis)
    {
        float cameraSpeed = Resources::currentRenderer->currentCamera->currentMovementSpeed * Resources::deltaTime;
        cameraSpeed = Resources::currentRenderer->currentCamera->currentMovementSpeed * Resources::deltaTime;
        Resources::currentRenderer->currentCamera->orientation.cameraPosition -= (cameraSpeed * axis) * Resources::currentRenderer->currentCamera->orientation.cameraFront;
        return false;
    }

    bool CAMERA_YAW_STICK (float axis)
    {
        Resources::currentRenderer->currentCamera->eulerAngle.yaw += axis;
        return true;
    }
        
    bool CAMERA_PITCH_STICK (float axis)
    {
        Resources::currentRenderer->currentCamera->eulerAngle.pitch -= axis;
        if (Resources::currentRenderer->currentCamera->eulerAngle.pitch > 89.0f)
            Resources::currentRenderer->currentCamera->eulerAngle.pitch = 89.0f;
        if (Resources::currentRenderer->currentCamera->eulerAngle.pitch < -89.0f)
            Resources::currentRenderer->currentCamera->eulerAngle.pitch = -89.0f;
        return true;
    }
        
    void CAMERA_FOV_TRIGGER (float axis)
    {
        if (axis < -0.15)
        {
            Resources::currentRenderer->currentCamera->fov = 45.0f;
        }
        if (axis < -0.5f)
        {
            Resources::currentRenderer->currentCamera->fov = 34.0f;
        }
        else if (axis < 0.0f)
        {
            Resources::currentRenderer->currentCamera->fov = 23.0f;
        }
        else if (axis < 0.5f)
        {
            Resources::currentRenderer->currentCamera->fov = 12.0f;
        }
        else if (axis < 0.75f)
        {
            Resources::currentRenderer->currentCamera->fov = 6.5f;
        }
        else if (axis < 0.875f)
        {
            Resources::currentRenderer->currentCamera->fov = 3.75f;
        }
        else
        {
            Resources::currentRenderer->currentCamera->fov = 1.0f;
        }
    }

    void CAMERA_SPEED_TRIGGER (float axis)
    {
        if (axis < -0.15)
        {
            Resources::currentRenderer->currentCamera->setCurrentCameraSpeed(Resources::currentRenderer->currentCamera->defaultMovementSpeed);
        }
        if (axis < -0.5f)
        {
            Resources::currentRenderer->currentCamera->setCurrentCameraSpeed(Resources::currentRenderer->currentCamera->defaultMovementSpeed * 3.0f);
        }
        else if (axis < 0.0f)
        {
            Resources::currentRenderer->currentCamera->setCurrentCameraSpeed(Resources::currentRenderer->currentCamera->defaultMovementSpeed * 5.0f);
        }
        else if (axis < 0.5f)
        {
            Resources::currentRenderer->currentCamera->setCurrentCameraSpeed(Resources::currentRenderer->currentCamera->defaultMovementSpeed * 7.0f);
        }
        else if (axis < 0.75f)
        {
            Resources::currentRenderer->currentCamera->setCurrentCameraSpeed(Resources::currentRenderer->currentCamera->defaultMovementSpeed * 8.0f);
        }
        else if (axis < 0.875f)
        {
            Resources::currentRenderer->currentCamera->setCurrentCameraSpeed(Resources::currentRenderer->currentCamera->defaultMovementSpeed * 9.0f);
        }
        else
        {
            Resources::currentRenderer->currentCamera->setCurrentCameraSpeed(Resources::currentRenderer->currentCamera->defaultMovementSpeed * 10.0f);
        }
    }

   

}