#pragma once
#include "../Includes.h"
#include "../Cameras/Cameras.h"
#include "../Input/Input.h"
#include "../Objects/Objects.h"
#include "../Model/Model.h"
#include "../UserInterface/WindowFunctions.h"

/*============================================================================================================================================================================================
 * Application Class
 *============================================================================================================================================================================================*/


class Application 
{

public:
	GLFWwindow* window;
	//ImGuiIO& io;
	const char* programName;
	int GLFWVersion[2];
	glm::vec2 screenDimension;
	glm::vec3 backgroundColor;
public:
	std::vector<Object> allSimpleObjects;
	std::vector<LightSource*> allLightSources;
	GLInput* currentInput;
	GLInput* defaultInput;
	GLCamera* currentCamera;
	GLMesh* defaultMesh;
	Model* defaultModel;
	GLShader* defaultShader;
	UIWindow* currentWindow;
};

/*============================================================================================================================================================================================
 * ApplicationManager Class
 *============================================================================================================================================================================================*/


class ApplicationManager {
public:
	Application defaultApplication;
	Application application2D;
public:
	void CreateApplications();
	void DestroyApplications();

private:
	void CreateDefaultApplication(Application& application);
	void CreateApplication2D(Application& application);
	
	void DestroyApplication(Application& application);
};