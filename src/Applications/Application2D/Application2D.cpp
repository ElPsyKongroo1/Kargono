#include "../../Library/Rendering/Includes.h"
#include "../../Library/Rendering/Initialization.h"
#include "Rendering1.h"
#include "../../Library/Rendering/Library.h"
#include "../../Library/Rendering/Shaders/Shaders.h"
#include "../../Library/Rendering/Mesh/Meshes.h"
#include "../../Library/Rendering/Textures/Textures.h"
#include "../../Library/Game/Objects/Objects.h"

/*============================================================================================================================================================================================
 * Application Entry Point
 *============================================================================================================================================================================================*/


int Application2D()

{
	// Initialize GLFW context, Meshes, Shaders, and Textures

	FInitializeRenderer(); // GLFW, GLEW, and IMGui contexts
	FInitializeLibraryResources();

	Orientation orientation2{ glm::vec4(1.0f, 0.0f, 0.0f, 270.0f),
							  glm::vec3(0.0f, 0.0f, -3.0f),
							  glm::vec3(1.0f, 1.0f, 2.0f) };
	ModelRenderer renderer2{ ModelRenderer(orientation2,
		Resources::currentRenderer->defaultModel,
		Resources::currentRenderer->defaultShader) };
	Object object2{ orientation2, &renderer2 };
	Resources::currentRenderer->objectRenderBuffer.push_back(object2);


	// Main running Loop
	while (!glfwWindowShouldClose(Resources::currentRenderer->window))
	{
		// PreRendering
		FPreRendering();

		// Render
		FRendering1();
		FWindowRendering();

		// Post Rendering
		FPostRendering();
	}

	FTerminateLibraryResources();
	FTerminateRenderer();

	return 0;
}
