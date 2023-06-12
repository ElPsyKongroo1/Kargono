#include "../Library/Includes.h"
#include "../Library/Initialization.h"
#include "Rendering1.h"
#include "../Library/Library.h"
#include "../Library/Shaders/Shaders.h"
#include "../Library/Mesh/Meshes.h"
#include "../Library/Textures/Textures.h"
#include "../Library/Objects/Objects.h"

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
		Resources::currentApplication->defaultModel,
		Resources::currentApplication->defaultShader) };
	Object object2{ orientation2, &renderer2 };
	Resources::currentApplication->objectRenderBuffer.push_back(object2);


	// Main running Loop
	while (!glfwWindowShouldClose(Resources::currentApplication->window))
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
