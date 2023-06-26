#include "../../Library/Includes.h"

#include "../../Library/Library.h"
#include "../../Library/Rendering/Shaders/Shaders.h"
#include "../../Library/Rendering/Mesh/Meshes.h"
#include "../../Library/Rendering/Textures/Textures.h"
#include "../../Library/Application/Objects/Objects.h"

/*============================================================================================================================================================================================
 * Application Entry Point
 *============================================================================================================================================================================================*/


int Application2D()

{
	// Initialize GLFW context, Meshes, Shaders, and Textures

	Resources::currentApplication->renderer->init();
	Resources::currentApplication->recentInput = Resources::inputManager.default2DInput;
	Resources::currentApplication->currentInput = Resources::currentApplication->recentInput;
	Resources::currentApplication->renderer->setDefaultValues(Resources::modelManager.simpleBackpack,
		Resources::meshManager.cubeMesh,
		Resources::shaderManager.lightingShader,
		Resources::cameraManager.static2DCamera);

	Orientation orientation2{ glm::vec4(1.0f, 0.0f, 0.0f, 270.0f),
							  glm::vec3(0.0f, 0.0f, -3.0f),
							  glm::vec3(1.0f, 1.0f, 2.0f) };
	ModelRenderer* renderer2{ new ModelRenderer(orientation2,
		Resources::currentApplication->renderer->defaultModel,
		Resources::currentApplication->renderer->defaultShader) };
	Object* object2{ new Object(orientation2, renderer2, nullptr) };
	Resources::currentApplication->renderer->objectRenderBuffer.push_back(object2);

	Orientation orientation10{ glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
							  glm::vec3(1.0f, 1.5f, -3.0f),
							  glm::vec3(5.0f, 5.0f, 1.0f) };
	ShapeRenderer* renderer10{ new ShapeRenderer(orientation10) };
	Object* object10{ new Object(orientation10, renderer10, nullptr) };
	Resources::currentApplication->renderer->objectRenderBuffer.push_back(object10);


	// Main running Loop
	while (!glfwWindowShouldClose(Resources::currentApplication->renderer->window))
	{
		Resources::currentApplication->renderer->render();
	}

	Resources::currentApplication->renderer->close();

	return 0;
}
