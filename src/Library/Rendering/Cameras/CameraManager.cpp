#include "Cameras.h"
#include "../Library.h"

/*============================================================================================================================================================================================
 *============================================================================================================================================================================================
 * Cameras Class
 *============================================================================================================================================================================================
 *============================================================================================================================================================================================*/

 /*============================================================================================================================================================================================
  * Instantiate/Delete GLCamera objects
  *============================================================================================================================================================================================*/

void CameraManager::CreateCameras()
{
    CreateFlyCamera();
    CreateStatic2DCamera();
}
void CameraManager::DestroyCameras()
{
    delete flyCamera;
    delete static2DCamera;
    flyCamera = nullptr;
    static2DCamera = nullptr;
}
/*============================================================================================================================================================================================
 * Initializes GLCamera Structs with specified values
 *============================================================================================================================================================================================*/
void CameraManager::CreateFlyCamera()
{
    flyCamera = new GLCamera();
    flyCamera->orientation = { glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f) };
    flyCamera->eulerAngle.yaw = -90.0f;
    flyCamera->eulerAngle.pitch = 0.0f;
    flyCamera->eulerAngle.roll = 0.0f;
    flyCamera->defaultMovementSpeed = 4.0f;
    flyCamera->currentMovementSpeed = flyCamera->defaultMovementSpeed;
    flyCamera->currentDiagonalMovementSpeed = sqrt(2 * flyCamera->currentMovementSpeed * flyCamera->currentMovementSpeed) / 2;
    flyCamera->projection = GLCamera::PERSPECTIVE;
    flyCamera->fov = 45.0f;
    flyCamera->lastX = 400.0f;
    flyCamera->lastY = 300.0f;
    flyCamera->firstMouse = true;
    flyCamera->defaultPanningSpeed = 0.1f;
    flyCamera->currentPanningSpeed = flyCamera->defaultPanningSpeed;
    flyCamera->flashLight = LightSource();
    flyCamera->flashLight.lightType = LightSource::SPOT;
    flyCamera->flashLight.parentObject = LightSource::CAMERA;
    flyCamera->flashLight.attenuation = glm::vec3(1.0f, 0.0014f, 0.000007f);
    if (Resources::currentRenderer->currentCamera == flyCamera)
    {
        Resources::currentRenderer->lightSourceRenderBuffer.push_back(&flyCamera->flashLight);
    }
    flyCamera->isFlashLight = true;

}

void CameraManager::CreateStatic2DCamera()
{
    static2DCamera = new GLCamera();
    static2DCamera->orientation = { glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f) };
    static2DCamera->eulerAngle.yaw = -90.0f;
    static2DCamera->eulerAngle.pitch = 0.0f;
    static2DCamera->eulerAngle.roll = 0.0f;
    static2DCamera->defaultMovementSpeed = 1.0f;
    static2DCamera->currentMovementSpeed = static2DCamera->defaultMovementSpeed;
    static2DCamera->currentDiagonalMovementSpeed = sqrt(2 * static2DCamera->currentMovementSpeed * static2DCamera->currentMovementSpeed) / 2;
    static2DCamera->projection = GLCamera::ORTHOGRAPHIC;
    static2DCamera->fov = 45.0f;
    static2DCamera->lastX = 400.0f;
    static2DCamera->lastY = 300.0f;
    static2DCamera->firstMouse = true;
    static2DCamera->defaultPanningSpeed = 0.1f;
    static2DCamera->currentPanningSpeed = static2DCamera->defaultPanningSpeed;
    static2DCamera->flashLight = LightSource();
    static2DCamera->flashLight.lightType = LightSource::SPOT;
    static2DCamera->flashLight.parentObject = LightSource::CAMERA;
    static2DCamera->flashLight.attenuation = glm::vec3(1.0f, 0.0014f, 0.000007f);
    if (Resources::currentRenderer->currentCamera == static2DCamera)
    {
        Resources::currentRenderer->lightSourceRenderBuffer.push_back(&static2DCamera->flashLight);
    }
    static2DCamera->isFlashLight = false;
}