#include "Breakout.h"
#include "../../Library/Includes.h"

#include "../../Library/Library.h"
#include "../../Library/Rendering/Shaders/Shaders.h"
#include "../../Library/Rendering/Mesh/Meshes.h"
#include "../../Library/Rendering/Textures/Textures.h"
#include "../../Library/Application/Objects/Objects.h"
void initializeRenderer();

void BreakoutStart()
{
	// Initialize GLFW context, Meshes, Shaders, and Textures
    initializeRenderer();
    Resources::currentApplication->defaultInput = Resources::inputManager.default3DInput;
    Resources::currentApplication->currentInput = Resources::currentApplication->defaultInput;

	Orientation orientation2{ glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
							  glm::vec3(0.0f, 0.0f, -0.0f),
							  glm::vec3(200.0f, 200.0f, 0.5f) };
	ShapeRenderer renderer2{ ShapeRenderer(orientation2,
		Resources::meshManager.cubeMesh,
		Resources::currentApplication->renderer->defaultShader) };
	Object object2{ orientation2, &renderer2 };
	Resources::currentApplication->renderer->objectRenderBuffer.push_back(object2);


	// Main running Loop
	while (!glfwWindowShouldClose(Resources::currentApplication->renderer->window))
	{
		Resources::currentApplication->renderer->render();
	}
	delete Resources::currentApplication->renderer->currentCamera;
	Resources::currentApplication->renderer->currentCamera = nullptr;

	Resources::currentApplication->renderer->close();
}

void initializeRenderer() 
{
    Resources::currentApplication->renderer->init();
    
	Resources::currentApplication->renderer->currentCamera = new GLCamera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(-90.0f, 0.0f, 0.0f), 3.0f, GLCamera::PERSPECTIVE,
		glm::vec2(-400.0f, 400.0f), glm::vec2(-300.0f, 300.0f), glm::vec2(-1.0f, 10.0f), 45.0f,
		(float)Resources::currentApplication->renderer->screenDimension.x / (float)Resources::currentApplication->renderer->screenDimension.y,
		0.1f);
    
    Resources::currentApplication->renderer->setDefaultValues(Resources::modelManager.simpleBackpack,
        Resources::meshManager.cubeMesh,
        Resources::shaderManager.defaultShader,
		Resources::currentApplication->renderer->currentCamera);
}