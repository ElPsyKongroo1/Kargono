#include "Includes.h"
#include "Library.h"
#include "Objects/Objects.h"

void FTerminateLibraryResources()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	Resources::meshManager.DestroyMeshes();
	Resources::textureManager.DestroyTextures();
	Resources::currentApplication->objectRenderBuffer.clear();
	Resources::currentApplication->lightSourceRenderBuffer.clear();
	Resources::cameraManager.DestroyCameras();
	Resources::inputManager.DestroyInputs();
	Resources::windowManager.DestroyUIWindows();
	Resources::modelManager.DestroyModels();
}
void FTerminateRenderer()
{

	glfwTerminate();
}