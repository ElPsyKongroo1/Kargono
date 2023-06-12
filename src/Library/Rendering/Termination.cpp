#include "Includes.h"
#include "Library.h"
#include "../Game/Objects/Objects.h"

void FTerminateLibraryResources()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	Resources::meshManager.DestroyMeshes();
	Resources::textureManager.DestroyTextures();
	Resources::currentRenderer->objectRenderBuffer.clear();
	Resources::currentRenderer->lightSourceRenderBuffer.clear();
	Resources::cameraManager.DestroyCameras();
	Resources::inputManager.DestroyInputs();
	Resources::windowManager.DestroyUIWindows();
	Resources::modelManager.DestroyModels();
}
void FTerminateRenderer()
{

	glfwTerminate();
}