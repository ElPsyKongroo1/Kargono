#include "../Input.h"
#include "../../Library.h"
#include "../MiscFunctions.h"

namespace Default3DFunctions {
    // Mouse Movement
    void CAMERA_YAW_PITCH_MOUSE(double xpos, double ypos)
    {
        float xoffset;
        float yoffset;
        glm::vec3 direction;

        if (Resources::currentApplication->currentCamera->firstMouse) // initially set to true
        {
            Resources::currentApplication->currentCamera->lastX = xpos;
            Resources::currentApplication->currentCamera->lastY = ypos;
            Resources::currentApplication->currentCamera->firstMouse = false;
        }

        xoffset = xpos - Resources::currentApplication->currentCamera->lastX;
        yoffset = Resources::currentApplication->currentCamera->lastY - ypos; // reversed since y-coordinates range from bottom to top
        Resources::currentApplication->currentCamera->lastX = xpos;
        Resources::currentApplication->currentCamera->lastY = ypos;

        xoffset *= Resources::currentApplication->currentCamera->currentPanningSpeed;
        yoffset *= Resources::currentApplication->currentCamera->currentPanningSpeed;

        Resources::currentApplication->currentCamera->eulerAngle.yaw += xoffset;
        Resources::currentApplication->currentCamera->eulerAngle.pitch += yoffset;

        if (Resources::currentApplication->currentCamera->eulerAngle.pitch > 89.0f)
            Resources::currentApplication->currentCamera->eulerAngle.pitch = 89.0f;
        if (Resources::currentApplication->currentCamera->eulerAngle.pitch < -89.0f)
            Resources::currentApplication->currentCamera->eulerAngle.pitch = -89.0f;


        direction.x = cos(glm::radians(Resources::currentApplication->currentCamera->eulerAngle.yaw)) * cos(glm::radians(Resources::currentApplication->currentCamera->eulerAngle.pitch));
        direction.y = sin(glm::radians(Resources::currentApplication->currentCamera->eulerAngle.pitch));
        direction.z = sin(glm::radians(Resources::currentApplication->currentCamera->eulerAngle.yaw)) * cos(glm::radians(Resources::currentApplication->currentCamera->eulerAngle.pitch));
        Resources::currentApplication->currentCamera->orientation.cameraFront = glm::normalize(direction);
    }
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

    bool CAMERA_DEINCREMENT_SENSITIVITY(GLInputLink* gamePadButton)
    {
        if ((Resources::currentApplication->currentCamera->currentPanningSpeed - Resources::currentApplication->currentCamera->defaultPanningSpeed) < Resources::currentApplication->currentCamera->defaultPanningSpeed)
            return false;
        Resources::currentApplication->currentCamera->currentPanningSpeed -= Resources::currentApplication->currentCamera->defaultPanningSpeed;
        return false;
    }

    bool CAMERA_INCREMENT_SENSITIVITY(GLInputLink* gamePadButton)
    {
        if ((Resources::currentApplication->currentCamera->currentPanningSpeed + Resources::currentApplication->currentCamera->defaultPanningSpeed) > (Resources::currentApplication->currentCamera->defaultPanningSpeed * 10))
            return false;
        Resources::currentApplication->currentCamera->currentPanningSpeed += Resources::currentApplication->currentCamera->defaultPanningSpeed;
        return false;
    }
    bool CAMERA_DEINCREMENT_SPEED(GLInputLink* gamePadButton)
    {
        if ((Resources::currentApplication->currentCamera->currentMovementSpeed - Resources::currentApplication->currentCamera->defaultMovementSpeed) < Resources::currentApplication->currentCamera->defaultMovementSpeed)
            return false;
        Resources::currentApplication->currentCamera->setCurrentCameraSpeed(Resources::currentApplication->currentCamera->currentMovementSpeed - Resources::currentApplication->currentCamera->defaultMovementSpeed);
        return false;

    }
    bool CAMERA_INCREMENT_SPEED(GLInputLink* gamePadButton)
    {
        if ((Resources::currentApplication->currentCamera->currentMovementSpeed + Resources::currentApplication->currentCamera->defaultMovementSpeed) > (Resources::currentApplication->currentCamera->defaultMovementSpeed * 50))
            return false;
        Resources::currentApplication->currentCamera->setCurrentCameraSpeed(Resources::currentApplication->currentCamera->currentMovementSpeed + Resources::currentApplication->currentCamera->defaultMovementSpeed);
        return false;
    }

    bool MOVE_FORWARD_KEY()
    {
        float cameraSpeed;
        cameraSpeed = Resources::currentApplication->currentCamera->currentMovementSpeed * Resources::deltaTime;
        Resources::currentApplication->currentCamera->orientation.cameraPosition += cameraSpeed * Resources::currentApplication->currentCamera->orientation.cameraFront;
        return false;
    }

    bool MOVE_BACKWARD_KEY()
    {
        float cameraSpeed;
        cameraSpeed = Resources::currentApplication->currentCamera->currentMovementSpeed * Resources::deltaTime;
        Resources::currentApplication->currentCamera->orientation.cameraPosition -= cameraSpeed * Resources::currentApplication->currentCamera->orientation.cameraFront;
        return false;
    }

    bool MOVE_LEFT_KEY () 
    {
        float cameraSpeed;
        cameraSpeed = Resources::currentApplication->currentCamera->currentMovementSpeed * Resources::deltaTime;
        Resources::currentApplication->currentCamera->orientation.cameraPosition -= glm::normalize(glm::cross(Resources::currentApplication->currentCamera->orientation.cameraFront, Resources::currentApplication->currentCamera->orientation.cameraUp)) * cameraSpeed;
        return false;
    }

    bool MOVE_RIGHT_KEY () 
    {
        float cameraSpeed;
        cameraSpeed = Resources::currentApplication->currentCamera->currentMovementSpeed * Resources::deltaTime;
        Resources::currentApplication->currentCamera->orientation.cameraPosition += glm::normalize(glm::cross(Resources::currentApplication->currentCamera->orientation.cameraFront, Resources::currentApplication->currentCamera->orientation.cameraUp)) * cameraSpeed;
        return false;
    }

    bool CAMERA_PITCH_UP_KEY () 
    {
        Resources::currentApplication->currentCamera->eulerAngle.pitch += Resources::currentApplication->currentCamera->currentPanningSpeed;
        if (Resources::currentApplication->currentCamera->eulerAngle.pitch > 89.0f)
            Resources::currentApplication->currentCamera->eulerAngle.pitch = 89.0f;
        if (Resources::currentApplication->currentCamera->eulerAngle.pitch < -89.0f)
            Resources::currentApplication->currentCamera->eulerAngle.pitch = -89.0f;
        return true;
    }
        

    bool CAMERA_PITCH_DOWN_KEY () 
    {
        Resources::currentApplication->currentCamera->eulerAngle.pitch -= Resources::currentApplication->currentCamera->currentPanningSpeed;
        if (Resources::currentApplication->currentCamera->eulerAngle.pitch > 89.0f)
            Resources::currentApplication->currentCamera->eulerAngle.pitch = 89.0f;
        if (Resources::currentApplication->currentCamera->eulerAngle.pitch < -89.0f)
            Resources::currentApplication->currentCamera->eulerAngle.pitch = -89.0f;
        return true;
    }
        

    bool CAMERA_YAW_LEFT_KEY () 
    {
        Resources::currentApplication->currentCamera->eulerAngle.yaw -= Resources::currentApplication->currentCamera->currentPanningSpeed;
        return true;
    }
        

    bool CAMERA_YAW_RIGHT_KEY () 
    {
        Resources::currentApplication->currentCamera->eulerAngle.yaw += Resources::currentApplication->currentCamera->currentPanningSpeed;
        return true;
    }
   
    bool MOVE_LEFT_RIGHT_STICK (float axis)
    {
        float cameraSpeed = Resources::currentApplication->currentCamera->currentMovementSpeed * Resources::deltaTime;
        cameraSpeed = Resources::currentApplication->currentCamera->currentMovementSpeed * Resources::deltaTime;
        Resources::currentApplication->currentCamera->orientation.cameraPosition += glm::normalize(glm::cross(Resources::currentApplication->currentCamera->orientation.cameraFront, Resources::currentApplication->currentCamera->orientation.cameraUp)) * (cameraSpeed * axis);
        return false;
    }
    bool MOVE_UP_DOWN_STICK (float axis)
    {
        float cameraSpeed = Resources::currentApplication->currentCamera->currentMovementSpeed * Resources::deltaTime;
        cameraSpeed = Resources::currentApplication->currentCamera->currentMovementSpeed * Resources::deltaTime;
        Resources::currentApplication->currentCamera->orientation.cameraPosition -= (cameraSpeed * axis) * Resources::currentApplication->currentCamera->orientation.cameraFront;
        return false;
    }

    bool CAMERA_YAW_STICK (float axis)
    {
        Resources::currentApplication->currentCamera->eulerAngle.yaw += axis;
        return true;
    }
        
    bool CAMERA_PITCH_STICK (float axis)
    {
        Resources::currentApplication->currentCamera->eulerAngle.pitch -= axis;
        if (Resources::currentApplication->currentCamera->eulerAngle.pitch > 89.0f)
            Resources::currentApplication->currentCamera->eulerAngle.pitch = 89.0f;
        if (Resources::currentApplication->currentCamera->eulerAngle.pitch < -89.0f)
            Resources::currentApplication->currentCamera->eulerAngle.pitch = -89.0f;
        return true;
    }
        
    void CAMERA_FOV_TRIGGER (float axis)
    {
        if (axis < -0.15)
        {
            Resources::currentApplication->currentCamera->fov = 45.0f;
        }
        if (axis < -0.5f)
        {
            Resources::currentApplication->currentCamera->fov = 34.0f;
        }
        else if (axis < 0.0f)
        {
            Resources::currentApplication->currentCamera->fov = 23.0f;
        }
        else if (axis < 0.5f)
        {
            Resources::currentApplication->currentCamera->fov = 12.0f;
        }
        else if (axis < 0.75f)
        {
            Resources::currentApplication->currentCamera->fov = 6.5f;
        }
        else if (axis < 0.875f)
        {
            Resources::currentApplication->currentCamera->fov = 3.75f;
        }
        else
        {
            Resources::currentApplication->currentCamera->fov = 1.0f;
        }
    }

    void CAMERA_SPEED_TRIGGER (float axis)
    {
        if (axis < -0.15)
        {
            Resources::currentApplication->currentCamera->setCurrentCameraSpeed(Resources::currentApplication->currentCamera->defaultMovementSpeed);
        }
        if (axis < -0.5f)
        {
            Resources::currentApplication->currentCamera->setCurrentCameraSpeed(Resources::currentApplication->currentCamera->defaultMovementSpeed * 3.0f);
        }
        else if (axis < 0.0f)
        {
            Resources::currentApplication->currentCamera->setCurrentCameraSpeed(Resources::currentApplication->currentCamera->defaultMovementSpeed * 5.0f);
        }
        else if (axis < 0.5f)
        {
            Resources::currentApplication->currentCamera->setCurrentCameraSpeed(Resources::currentApplication->currentCamera->defaultMovementSpeed * 7.0f);
        }
        else if (axis < 0.75f)
        {
            Resources::currentApplication->currentCamera->setCurrentCameraSpeed(Resources::currentApplication->currentCamera->defaultMovementSpeed * 8.0f);
        }
        else if (axis < 0.875f)
        {
            Resources::currentApplication->currentCamera->setCurrentCameraSpeed(Resources::currentApplication->currentCamera->defaultMovementSpeed * 9.0f);
        }
        else
        {
            Resources::currentApplication->currentCamera->setCurrentCameraSpeed(Resources::currentApplication->currentCamera->defaultMovementSpeed * 10.0f);
        }
    }

   

}