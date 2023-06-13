#pragma once
#include "../../Rendering/Includes.h"
#include "../../Rendering/Textures/Textures.h"
#include "../../Rendering/Shaders/Shaders.h"
#include "../../Rendering/Mesh/Meshes.h"
#include "../../Rendering/Model/Model.h"
#include "../../Rendering/LightSource/LightSource.h"



struct Orientation 
{
	glm::vec4 rotation; // x,y,z, and rotation in degrees
	glm::vec3 translation;
	glm::vec3 scale;
};

class Renderable
{
public:
	LightSource lightSource;
	Orientation* orientation{nullptr};
	GLShader* shader;
public:
	virtual void render() = 0;
	virtual void addLightSource(glm::vec3 color) = 0;
};

