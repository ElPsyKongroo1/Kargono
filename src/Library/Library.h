#pragma once
#include "Shaders/Shaders.h"
#include "Mesh/Meshes.h"
#include "../Library/Textures/Textures.h"
#include "Objects/Objects.h"
#include "Cameras/Cameras.h"
#include "Input/Input.h"
#include "LightSource/LightSource.h"
#include "Application/Application.h"
#include "Model/Model.h"

namespace Resources 
{
	extern InputManager inputManager;
	extern TextureManager textureManager;
	extern MeshManager meshManager;
	extern ShaderManager shaderManager;
	extern CameraManager cameraManager;
	extern ApplicationManager applicationManager;
	extern WindowManager windowManager;
	extern ModelManager modelManager;

	extern Application* currentApplication;

	extern float currentFrame;
	extern float deltaTime;
	extern float lastFrame;
	extern int framesPerSecond;
	extern float framesCounter;
}