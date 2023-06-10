#include "Input.h"
#include "../Library.h"
#include "InputFunctions.h"


void CAMERA_YAW_PITCH_MOUSEF(double xpos, double ypos) 
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