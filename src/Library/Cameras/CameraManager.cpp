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
    camera.cameraPosition = glm::vec3();
    camera.cameraFront = glm::vec3();
    camera.cameraUp = glm::vec3();

    camera.currentCameraSpeed = -1;
    camera.slowCameraSpeed = -1;
    camera.currentSensitivity = -1;
    camera.slowSensitivity = -1;

    camera.pitch = -1;
    camera.yaw = -1;
    camera.fov = -1;
    camera.lastX = -1;
    camera.lastY = -1;
    camera.firstMouse = 0;
    camera.isFlashLight = 0;
    camera.projection = GLCamera::NOPROJECTION;
    camera.flashLight = LightSource();
}
/*============================================================================================================================================================================================
 * Initializes GLCamera Structs with specified values
 *============================================================================================================================================================================================*/
void CameraManager::CreateFlyCamera(GLCamera& camera)
{
    camera.cameraPosition = glm::vec3(0.0f, 0.0f, 3.0f);
    camera.cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    camera.cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    camera.slowCameraSpeed = 4.0f;
    camera.currentCameraSpeed = camera.slowCameraSpeed;
    camera.currentCameraDiagonal = sqrt(2 * camera.currentCameraSpeed * camera.currentCameraSpeed) / 2;
    camera.projection = GLCamera::PERSPECTIVE;
    camera.yaw = -90.0f;
    camera.pitch = 0.0f;
    camera.fov = 45.0f;
    camera.lastX = 400.0f;
    camera.lastY = 300.0f;
    camera.firstMouse = true;
    camera.slowSensitivity = 0.1f;
    camera.currentSensitivity = camera.slowSensitivity;
    camera.flashLight = LightSource();
    camera.flashLight.lightType = LightSource::SPOT;
    camera.flashLight.parentObject = LightSource::CAMERA;
    camera.flashLight.attenuation = glm::vec3(1.0f, 0.0014f, 0.000007f);
    if (Resources::currentApplication->currentCamera == &camera)
    {
        Resources::currentApplication->lightSourceRenderBuffer.push_back(&camera.flashLight);
    }
    camera.isFlashLight = true;

}

void CameraManager::CreateStatic2DCamera(GLCamera& camera)
{
    camera.cameraPosition = glm::vec3(0.0f, 0.0f, 0.0f);
    camera.cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    camera.cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    camera.slowCameraSpeed = 1.0f;
    camera.currentCameraSpeed = camera.slowCameraSpeed;
    camera.currentCameraDiagonal = sqrt(2 * camera.currentCameraSpeed * camera.currentCameraSpeed) / 2;
    camera.projection = GLCamera::ORTHOGRAPHIC;
    camera.yaw = -90.0f;
    camera.pitch = 0.0f;
    camera.fov = 45.0f;
    camera.lastX = 400.0f;
    camera.lastY = 300.0f;
    camera.firstMouse = true;
    camera.slowSensitivity = 0.1f;
    camera.currentSensitivity = camera.slowSensitivity;
    camera.flashLight = LightSource();
    camera.flashLight.lightType = LightSource::SPOT;
    camera.flashLight.parentObject = LightSource::CAMERA;
    camera.flashLight.attenuation = glm::vec3(1.0f, 0.0014f, 0.000007f);
    if (Resources::currentApplication->currentCamera == &camera)
    {
        Resources::currentApplication->lightSourceRenderBuffer.push_back(&camera.flashLight);
    }
    camera.isFlashLight = false;
}