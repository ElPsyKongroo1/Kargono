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
	struct EulerAngle 
	{
		float yaw;
		float pitch;
		float roll;
	};
public:
	enum ProjectionType
	{
		PERSPECTIVE, ORTHOGRAPHIC, NOPROJECTION
	};

public:
	// Orientation
	Orientation orientation;
	// Euler Angles
	EulerAngle eulerAngle;

	// Movement Speed
	float currentMovementSpeed, defaultMovementSpeed, currentDiagonalMovementSpeed;
	float currentPanningSpeed, defaultPanningSpeed;
	
	

	// State
	ProjectionType projection;
	float fov;
	LightSource flashLight;
	bool isFlashLight;

	// Other
	float lastX, lastY;
	bool firstMouse;
public:
	void setCurrentCameraSpeed(float speed)
	{
		currentMovementSpeed = speed;
		currentDiagonalMovementSpeed = sqrt(2 * currentMovementSpeed * currentMovementSpeed) / 2;
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
