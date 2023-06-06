#pragma once
#include "../Includes.h"
#include "../Textures/Textures.h"
#include "../Shaders/Shaders.h"
#include "../Mesh/Meshes.h"
#include "../Model/Model.h"
#include "../LightSource/LightSource.h"
#include "RenderableInterface.h"

/*============================================================================================================================================================================================
 * Object Class
 *============================================================================================================================================================================================*/

class Object : public Renderable
{
public:
	glm::vec4 rotation; // x,y,z, and rotation in degrees
	glm::vec3 translation;
	glm::vec3 scale;
	LightSource lightSource;
	bool isLightSource;
	Model* model;
	GLShader* shader;
	

public:
	Object(const glm::vec4& rotation,
		const glm::vec3& translation,
		const glm::vec3& scale,
		Model* model,
		GLShader* shader);
	Object(const glm::vec4& rotation,
		const glm::vec3& translation,
		const glm::vec3& scale);
public:
	void createLightSource(glm::vec3 color);
	void render() override {}
private:

};
