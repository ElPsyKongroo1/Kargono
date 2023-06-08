#include "Includes.h"
#include "Shaders/Shaders.h"
#include "Mesh/Meshes.h"
#include "Textures/Textures.h"
#include "Objects/Objects.h"
#include "Cameras/Cameras.h"
#include "Input/Input.h"
#include "LightSource/LightSource.h"
#include "RendererState/RendererState.h"
#include "UserInterface/WindowFunctions.h"
#include "Model/Model.h"

namespace Resources 
{
/*============================================================================================================================================================================================
 * Static Resources
 *============================================================================================================================================================================================*/
	RendererStateManager applicationManager = RendererStateManager();
	RendererState* currentApplication = &applicationManager.defaultApplication;
	ShaderManager shaderManager = ShaderManager();
	MeshManager meshManager = MeshManager();
	TextureManager textureManager = TextureManager();
	CameraManager cameraManager = CameraManager();
	InputManager inputManager = InputManager();
	WindowManager windowManager = WindowManager();
	ModelManager modelManager = ModelManager();
	

	

/*============================================================================================================================================================================================
 * Manage Update Time
 *============================================================================================================================================================================================*/
	float deltaTime = 0.0f;
	float lastFrame = 0.0f;
	float currentFrame;
	int framesPerSecond = 0;
	float framesCounter = 0.0f;
}
