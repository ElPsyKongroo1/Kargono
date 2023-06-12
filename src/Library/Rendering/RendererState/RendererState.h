#pragma once
#include "../Includes.h"
#include "../Cameras/Cameras.h"
#include "../../Game/Input/Input.h"
#include "../../Game/Objects/Objects.h"
#include "../Model/Model.h"
#include "../UserInterface/WindowFunctions.h"

/*============================================================================================================================================================================================
 * RendererState Class
 *============================================================================================================================================================================================*/


class RendererState 
{
public:
	void setDefaultValues(GLInput& input, Model* model, GLMesh& mesh, GLShader* shader, GLCamera& camera);
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
 * RendererStateManager Class
 *============================================================================================================================================================================================*/


class RendererStateManager {
public:
	RendererState Sample3DRenderer;
	RendererState Sample2DRenderer;
public:
	void CreateDefaultRenderers();
	void DestroyDefaultRenderers();

private:
	void CreateSample3DRenderer(RendererState& application);
	void CreateSample2DRenderer(RendererState& application);
	
	void DestroyRenderer(RendererState& application);
};