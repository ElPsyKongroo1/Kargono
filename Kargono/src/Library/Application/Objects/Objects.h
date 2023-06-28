#pragma once

#include "../../Rendering/Renderable/RenderableInterface.h"
#include "../Audio/AudioContext.h"

/*============================================================================================================================================================================================
 * Object Class
 *============================================================================================================================================================================================*/

class ShapeRenderer : public Renderable
{
public:
	GLMesh* mesh;

public:
	ShapeRenderer(Orientation& orientation,
		GLMesh* mesh,
		GLShader* shader);
	ShapeRenderer(Orientation& orientation);
	~ShapeRenderer();
public:
	void addLightSource(glm::vec3 color);
	void render() override 
	{
		this->mesh->Draw(this);
	}
private:

};

class ModelRenderer : public Renderable
{
public:
	Model* model;

public:
	ModelRenderer(Orientation& orientation,
		Model* model,
		GLShader* shader);
	ModelRenderer(Orientation& orientation);
	~ModelRenderer();
public:
	void addLightSource(glm::vec3 color);
	void render() override
	{
		this->model->Draw(this);
	}
private:

};


class Object 
{
public:
	Object(Orientation orientation, Renderable* renderer, AudioSource* audioSource) : orientation{ orientation }, renderer{ renderer }, audioSource{audioSource}
	{
		
		assert(renderer && "Object class failed to instantiate. Renderer is a nullptr");
		renderer->orientation = &this->orientation;
	}
	~Object() 
	{
		delete renderer;
		renderer = nullptr;
	}

public:
	Orientation orientation;
	Renderable* renderer;
	AudioSource* audioSource;
};