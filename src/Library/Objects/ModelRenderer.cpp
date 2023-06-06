#include "Objects.h"
#include "../Library.h"

/*============================================================================================================================================================================================
 *============================================================================================================================================================================================
 * ModelRenderer Class
 *============================================================================================================================================================================================
 *============================================================================================================================================================================================*/

 /*============================================================================================================================================================================================
  * Overloaded Constructors
  *============================================================================================================================================================================================*/

ModelRenderer::ModelRenderer(const glm::vec4& rotation,
	const glm::vec3& translation,
	const glm::vec3& scale,
	Model* model,
	GLShader* shader)

{
	/*this->orientation->rotation = rotation;
	this->orientation->translation = translation;
	this->orientation->scale = scale;*/
	this->rotation = rotation;
	this->translation = translation;
	this->scale = scale;
	this->model = model;
	this->shader = shader;
	this->isLightSource = false;

}

ModelRenderer::ModelRenderer(const glm::vec4& rotation,
	const glm::vec3& translation,
	const glm::vec3& scale)
{
	/*this->orientation->rotation = rotation;
	this->orientation->translation = translation;
	this->orientation->scale = scale;*/
	this->rotation = rotation;
	this->translation = translation;
	this->scale = scale;
	this->model = Resources::currentApplication->defaultModel;
	this->shader = Resources::currentApplication->defaultShader;
	this->isLightSource = false;

}

/*============================================================================================================================================================================================
 * Getter/Setter
 *============================================================================================================================================================================================*/

void ModelRenderer::createLightSource(glm::vec3 color)
{
	isLightSource = true;
	this->lightSource = LightSource(this->translation, color);
	this->shader = &Resources::shaderManager.lightSourceShader;
	lightSource.parentObject = LightSource::OBJECT;
	Resources::currentApplication->allLightSources.push_back(&lightSource);
}


/*============================================================================================================================================================================================
 * External Functionality
 *============================================================================================================================================================================================*/

