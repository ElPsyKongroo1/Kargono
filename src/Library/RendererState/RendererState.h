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


class RendererState 
{

public:
	GLFWwindow* window;
	//ImGuiIO& io;
	const char* programName;
	int GLFWVersion[2];
	glm::vec2 screenDimension;
	glm::vec3 backgroundColor;
public:
	std::vector<Object> objectRenderBuffer;
	std::vector<LightSource*> lightSourceRenderBuffer;
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


class RendererStateManager {
public:
	RendererState defaultApplication;
	RendererState application2D;
public:
	void CreateApplications();
	void DestroyApplications();

private:
	void CreateDefaultApplication(RendererState& application);
	void CreateApplication2D(RendererState& application);
	
	void DestroyApplication(RendererState& application);
};