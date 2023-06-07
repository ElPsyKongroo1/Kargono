#pragma once
#include "../Includes.h"



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
protected:
	virtual void render() = 0;
public:
	virtual void addLightSource(glm::vec3 color) = 0;

};