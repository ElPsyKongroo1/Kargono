#include "Includes.h"
#include "Rendering/Shaders/Shaders.h"
#include "Rendering/Mesh/Meshes.h"
#include "Rendering/Textures/Textures.h"
#include "Application/Objects/Objects.h"
#include "Rendering/Cameras/Cameras.h"
#include "Application/Input/Input.h"
#include "Rendering/LightSource/LightSource.h"
#include "Rendering/RendererState/RendererState.h"
#include "Rendering/UserInterface/WindowFunctions.h"
#include "Rendering/Model/Model.h"

#include "Application/ApplicationInterface/ApplicationManager.h"

namespace Resources 
{
/*============================================================================================================================================================================================
 * Static Resources
 *============================================================================================================================================================================================*/
	
	Application* currentApplication{ nullptr };
	GameApplication* currentGame{ nullptr };

	ApplicationManager applicationManager = ApplicationManager();
	RendererStateManager rendererManager = RendererStateManager();
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
	float deltaTime{0.0f};
	float lastFrame{0.0f};
	float currentFrame;
	int framesPerSecond{0};
	float framesCounter{0.0f};
}
