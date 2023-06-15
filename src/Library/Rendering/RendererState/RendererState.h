#pragma once
#include "../../Includes.h"
#include "../Cameras/Cameras.h"
#include "../../Application/Input/Input.h"
#include "../../Application/Objects/Objects.h"
#include "../Model/Model.h"
#include "../UserInterface/WindowFunctions.h"

/*============================================================================================================================================================================================
 * RendererState Class
 *============================================================================================================================================================================================*/


class RendererState 
{
public:
	void init();
	void close();
	void render();
	void setDefaultValues(Model* model, GLMesh* mesh, GLShader* shader, GLCamera* camera);
private:
	void InitializeDefaultResources();
	void InitializeRenderer();

	void PreRendering();
	void WindowRendering();
	void PostRendering();

	void terminate();
	void closeLibraryResources();
public:
	RendererState() {}
	RendererState(const char* programName, int GLFWVersion[2], glm::vec2 screenDimension, glm::vec3 backgroundColor);
	~RendererState();
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
	RendererState* Sample3DRenderer;
	RendererState* Sample2DRenderer;
public:
	void CreateDefaultRenderers();
	void DestroyDefaultRenderers();

private:
	void CreateSample3DRenderer();
	void CreateSample2DRenderer();
	
	void DestroyRenderer(RendererState& application);
};