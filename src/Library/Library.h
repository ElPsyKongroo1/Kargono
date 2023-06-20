#pragma once
#include "Rendering/Shaders/Shaders.h"
#include "Rendering/Mesh/Meshes.h"
#include "Rendering/Textures/Textures.h"
#include "Application/Objects/Objects.h"
#include "Rendering/Cameras/Cameras.h"
#include "Application/Input/Input.h"
#include "Rendering/LightSource/LightSource.h"
#include "Rendering/RendererState/RendererState.h"

#include "Application/ApplicationInterface/ApplicationManager.h"
#include "Rendering/Model/Model.h"

namespace Resources 
{
	extern ApplicationManager applicationManager;
	extern InputManager inputManager;
	extern TextureManager textureManager;
	extern MeshManager meshManager;
	extern ShaderManager shaderManager;
	extern CameraManager cameraManager;
	extern RendererStateManager rendererManager;
	extern WindowManager windowManager;
	extern ModelManager modelManager;

	extern Application* currentApplication;
	extern GameApplication* currentGame;

	extern float currentFrame;
	extern float deltaTime;
	extern float lastFrame;
	extern int framesPerSecond;
	extern float framesCounter;
	extern long long runtime;
}