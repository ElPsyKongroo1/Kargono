#include "../../Library/Rendering/Includes.h"

#include "../../Library/Rendering/Library.h"
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

	Resources::currentRenderer->init();

	Resources::currentRenderer->setDefaultValues(Resources::inputManager.default2DInput,
		Resources::modelManager.simpleBackpack,
		Resources::meshManager.cubeMesh,
		Resources::shaderManager.lightingShader,
		Resources::cameraManager.static2DCamera);

	Orientation orientation2{ glm::vec4(1.0f, 0.0f, 0.0f, 270.0f),
							  glm::vec3(0.0f, 0.0f, -3.0f),
							  glm::vec3(1.0f, 1.0f, 2.0f) };
	ModelRenderer renderer2{ ModelRenderer(orientation2,
		Resources::currentRenderer->defaultModel,
		Resources::currentRenderer->defaultShader) };
	Object object2{ orientation2, &renderer2 };
	Resources::currentRenderer->objectRenderBuffer.push_back(object2);

	Orientation orientation10{ glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
							  glm::vec3(1.0f, 1.5f, -3.0f),
							  glm::vec3(1.0f, 1.0f, 1.0f) };
	ShapeRenderer renderer10{ ShapeRenderer(orientation10) };
	Object object10{ orientation10, &renderer10 };
	Resources::currentRenderer->objectRenderBuffer.push_back(object10);


	// Main running Loop
	while (!glfwWindowShouldClose(Resources::currentRenderer->window))
	{
		Resources::currentRenderer->render();
	}

	Resources::currentRenderer->close();

	return 0;
}
