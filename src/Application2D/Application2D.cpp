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
