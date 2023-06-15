#pragma once
#include "../../Includes.h"
#include "../LightSource/LightSource.h"



/*============================================================================================================================================================================================
 * Camera Class that represents individual camera object
 *============================================================================================================================================================================================*/
class GLCamera
{
public:
	enum ProjectionType
	{
		PERSPECTIVE, ORTHOGRAPHIC, NOPROJECTION
	};
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

	struct FrustrumDimensions
	{
		ProjectionType projection;
		glm::vec2 depthDimension;
		glm::vec2 widthDimension;
		glm::vec2 heightDimension;
		float fov;
		float aspectRatio;
	};


public:
	// Orientation
	Orientation orientation;
	// Euler Angles
	EulerAngle eulerAngle;
	// Dimensions for camera frustrum
	FrustrumDimensions frustrumDimensions;

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
	GLCamera() {}
	GLCamera(glm::vec3 cameraPosition, glm::vec3 cameraFront, glm::vec3 cameraUp,
		glm::vec3 eulerAngles, float movementSpeed, GLCamera::ProjectionType projection,
		glm::vec2 frustWidth, glm::vec2 frustHeight, glm::vec2 frustDepth, float fov, float aspectRatio,
		float panSpeed);
	~GLCamera() 
	{
		eulerAngle.yaw = -1;
		eulerAngle.pitch = -1;
		eulerAngle.roll = -1;

		orientation.cameraPosition = glm::vec3();
		orientation.cameraFront = glm::vec3();
		orientation.cameraUp = glm::vec3();

		currentMovementSpeed = -1;
		defaultMovementSpeed = -1;
		currentPanningSpeed = -1;
		defaultPanningSpeed = -1;

		frustrumDimensions.projection = GLCamera::NOPROJECTION;
		frustrumDimensions.widthDimension = glm::vec2();
		frustrumDimensions.heightDimension = glm::vec2();
		frustrumDimensions.depthDimension = glm::vec2();
		frustrumDimensions.fov = -1.0f;
		frustrumDimensions.aspectRatio = -1.0f;

		fov = -1;
		lastX = -1;
		lastY = -1;
		firstMouse = 0;
		isFlashLight = 0;
		projection = GLCamera::NOPROJECTION;
		flashLight = LightSource();
	}
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
	GLCamera* flyCamera;
	GLCamera* static2DCamera;
public:
	void CreateCameras();
	void DestroyCameras();
private:
	void CreateFlyCamera();
	void CreateStatic2DCamera();

};
