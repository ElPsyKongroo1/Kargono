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

class ModelRenderer : public Renderable
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
	ModelRenderer(const glm::vec4& rotation,
		const glm::vec3& translation,
		const glm::vec3& scale,
		Model* model,
		GLShader* shader);
	ModelRenderer(const glm::vec4& rotation,
		const glm::vec3& translation,
		const glm::vec3& scale);
public:
	void createLightSource(glm::vec3 color);
	void render() override {}
private:

};

//class PrimitiveRenderer : public Renderable
//{
//public:
//	glm::vec4 rotation; // x,y,z, and rotation in degrees
//	glm::vec3 translation;
//	glm::vec3 scale;
//	LightSource lightSource;
//	bool isLightSource;
//	Model* model;
//	GLShader* shader;
//
//
//public:
//	PrimitiveRenderer(const glm::vec4& rotation,
//		const glm::vec3& translation,
//		const glm::vec3& scale,
//		Model* model,
//		GLShader* shader);
//	PrimitiveRenderer(const glm::vec4& rotation,
//		const glm::vec3& translation,
//		const glm::vec3& scale);
//public:
//	void createLightSource(glm::vec3 color);
//	void render() override {}
//private:
//
//};