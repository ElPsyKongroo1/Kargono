#pragma once
#include "../Includes.h"
#include "../LightSource/LightSource.h"



/*============================================================================================================================================================================================
 * Camera Class that represents individual camera object
 *============================================================================================================================================================================================*/
class GLCamera
{
public:
	struct Orientation 
	{
		glm::vec3 cameraPosition;
		glm::vec3 cameraFront;
		glm::vec3 cameraUp;
	};
public:
	enum Projection
	{
		PERSPECTIVE, ORTHOGRAPHIC, NOPROJECTION
	};

public:
	Orientation orientation;
	float currentCameraSpeed, slowCameraSpeed, currentCameraDiagonal;
	float currentSensitivity, slowSensitivity;
	Projection projection;
	float yaw, pitch;
	float fov;
	float lastX, lastY;
	bool firstMouse;
	LightSource flashLight;
	bool isFlashLight;
public:
	void setCurrentCameraSpeed(float speed)
	{
		currentCameraSpeed = speed;
		currentCameraDiagonal = sqrt(2 * currentCameraSpeed * currentCameraSpeed) / 2;
	}
public:
	void processProjection(glm::mat4& view, glm::mat4& projection);
};

/*============================================================================================================================================================================================
 * Cameras class that manages GLCamera objects
 *============================================================================================================================================================================================*/
class CameraManager
{
public:
	GLCamera flyCamera;
	GLCamera static2DCamera;
public:
	void CreateCameras();
	void DestroyCameras();
private:
	void CreateFlyCamera(GLCamera& camera);
	void CreateStatic2DCamera(GLCamera& camera);
	void DestroyCamera(GLCamera& camera);

};
