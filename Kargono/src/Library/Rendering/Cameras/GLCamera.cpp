#include "Kargono/kgpch.h"
#include "Cameras.h"
#include "../../Library.h"

/*============================================================================================================================================================================================
 *============================================================================================================================================================================================
 * GLCamera Class
 *============================================================================================================================================================================================
 *============================================================================================================================================================================================*/

 /*===========================================================================e3=================================================================================================================
  * External Functionality
  *============================================================================================================================================================================================*/

GLCamera::GLCamera(glm::vec3 cameraPosition, glm::vec3 cameraFront, glm::vec3 cameraUp,
    glm::vec3 eulerAngles, float movementSpeed, GLCamera::ProjectionType projection,
    glm::vec2 frustWidth, glm::vec2 frustHeight, glm::vec2 frustDepth, float fov, float aspectRatio,
    float panSpeed)
{
   this->orientation = { cameraPosition, cameraFront, cameraUp };
   this->eulerAngle.yaw = eulerAngles.x;
   this->eulerAngle.pitch = eulerAngles.y;
   this->eulerAngle.roll = eulerAngles.z;
   this->defaultMovementSpeed = movementSpeed;
   this->currentMovementSpeed = this->defaultMovementSpeed;
   this->currentDiagonalMovementSpeed = sqrt(2 * this->currentMovementSpeed * this->currentMovementSpeed) / 2;
   this->projection = projection;
   this->frustrumDimensions.projection = projection;
   this->frustrumDimensions.widthDimension = frustWidth;
   this->frustrumDimensions.heightDimension = frustHeight;
   this->frustrumDimensions.depthDimension = frustDepth;
   this->frustrumDimensions.fov = fov;
   this->frustrumDimensions.aspectRatio = aspectRatio;
   this->fov = fov;
   this->lastX = 400.0f;
   this->lastY = 300.0f;
   this->firstMouse = true;
   this->defaultPanningSpeed = panSpeed;
   this->currentPanningSpeed = this->defaultPanningSpeed;
   this->flashLight = LightSource();
   this->flashLight.lightType = LightSource::SPOT;
   this->flashLight.parentObject = LightSource::CAMERA;
   this->flashLight.attenuation = glm::vec3(1.0f, 0.0014f, 0.000007f);
    if (Resources::currentApplication->renderer->currentCamera == this)
    {
        Resources::currentApplication->renderer->lightSourceRenderBuffer.push_back(&flashLight);
    }
    this->isFlashLight = false;
}


void GLCamera::processProjection(glm::mat4 &view, glm::mat4 &projection)
{
    view = glm::lookAt(orientation.cameraPosition, orientation.cameraPosition + orientation.cameraFront, orientation.cameraUp);
    if (this->projection == PERSPECTIVE)
    {
        projection = glm::perspective(glm::radians(frustrumDimensions.fov),
                                      frustrumDimensions.aspectRatio,
                                      frustrumDimensions.depthDimension.x,
                                      frustrumDimensions.depthDimension.y);
    }
    else if (this->projection == ORTHOGRAPHIC)
    {
        projection = glm::ortho(frustrumDimensions.widthDimension.x,
                                frustrumDimensions.widthDimension.y,
                                frustrumDimensions.heightDimension.x, 
                                frustrumDimensions.heightDimension.y, 
                                frustrumDimensions.depthDimension.x, 
                                frustrumDimensions.depthDimension.y);
    }
    else
    {
        std::cout << "ERROR::PROJECTION::PROJECTION_ENUM_INCORRECT" << std::endl;
        throw std::runtime_error("Check Logs");
    }
}
