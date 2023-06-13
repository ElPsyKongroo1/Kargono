#include "Includes.h"
#include "Library.h"
#include "../Game/Objects/Objects.h"

void FTerminateLibraryResources()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	Resources::meshManager.DestroyMeshes();
	Resources::shaderManager.DestroyShaders();
	Resources::textureManager.DestroyTextures();
	Resources::currentRenderer->objectRenderBuffer.clear();
	Resources::currentRenderer->lightSourceRenderBuffer.clear();
	Resources::cameraManager.DestroyCameras();
	Resources::inputManager.DestroyInputs();
	Resources::windowManager.DestroyUIWindows();
	Resources::modelManager.DestroyModels();

	Resources::deltaTime = 0.0f;
	Resources::lastFrame = 0.0f;
	Resources::currentFrame = 0.0f;
	Resources::framesPerSecond = 0;
	Resources::framesCounter = 0.0f;
}
void FTerminateRenderer()
{

	glfwTerminate();
}