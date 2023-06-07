#pragma once
#include "../Includes.h"
#include "../Includes.h"
#include "../Textures/Textures.h"
#include "../Shaders/Shaders.h"
#include "../Mesh/Meshes.h"
#include "../Model/Model.h"
#include "../LightSource/LightSource.h"



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

