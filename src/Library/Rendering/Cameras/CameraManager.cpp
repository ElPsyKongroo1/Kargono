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
    CreateFlyCamera(flyCamera);
    CreateStatic2DCamera(static2DCamera);
}
void CameraManager::DestroyCameras()
{
    DestroyCamera(flyCamera);
    DestroyCamera(static2DCamera);
}
void CameraManager::DestroyCamera(GLCamera& camera)
{
}
/*============================================================================================================================================================================================
 * Initializes GLCamera Structs with specified values
 *============================================================================================================================================================================================*/
void CameraManager::CreateFlyCamera(GLCamera& camera)
{
    camera.orientation = { glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f) };
    camera.eulerAngle.yaw = -90.0f;
    camera.eulerAngle.pitch = 0.0f;
    camera.eulerAngle.roll = 0.0f;
    camera.defaultMovementSpeed = 4.0f;
    camera.currentMovementSpeed = camera.defaultMovementSpeed;
    camera.currentDiagonalMovementSpeed = sqrt(2 * camera.currentMovementSpeed * camera.currentMovementSpeed) / 2;
    camera.projection = GLCamera::PERSPECTIVE;
    camera.fov = 45.0f;
    camera.lastX = 400.0f;
    camera.lastY = 300.0f;
    camera.firstMouse = true;
    camera.defaultPanningSpeed = 0.1f;
    camera.currentPanningSpeed = camera.defaultPanningSpeed;
    camera.flashLight = LightSource();
    camera.flashLight.lightType = LightSource::SPOT;
    camera.flashLight.parentObject = LightSource::CAMERA;
    camera.flashLight.attenuation = glm::vec3(1.0f, 0.0014f, 0.000007f);
    if (Resources::currentRenderer->currentCamera == &camera)
    {
        Resources::currentRenderer->lightSourceRenderBuffer.push_back(&camera.flashLight);
    }
    camera.isFlashLight = true;

}

void CameraManager::CreateStatic2DCamera(GLCamera& camera)
{
    camera.orientation = { glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f) };
    camera.eulerAngle.yaw = -90.0f;
    camera.eulerAngle.pitch = 0.0f;
    camera.eulerAngle.roll = 0.0f;
    camera.defaultMovementSpeed = 1.0f;
    camera.currentMovementSpeed = camera.defaultMovementSpeed;
    camera.currentDiagonalMovementSpeed = sqrt(2 * camera.currentMovementSpeed * camera.currentMovementSpeed) / 2;
    camera.projection = GLCamera::ORTHOGRAPHIC;
    camera.fov = 45.0f;
    camera.lastX = 400.0f;
    camera.lastY = 300.0f;
    camera.firstMouse = true;
    camera.defaultPanningSpeed = 0.1f;
    camera.currentPanningSpeed = camera.defaultPanningSpeed;
    camera.flashLight = LightSource();
    camera.flashLight.lightType = LightSource::SPOT;
    camera.flashLight.parentObject = LightSource::CAMERA;
    camera.flashLight.attenuation = glm::vec3(1.0f, 0.0014f, 0.000007f);
    if (Resources::currentRenderer->currentCamera == &camera)
    {
        Resources::currentRenderer->lightSourceRenderBuffer.push_back(&camera.flashLight);
    }
    camera.isFlashLight = false;
}