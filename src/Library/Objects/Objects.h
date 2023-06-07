#pragma once

#include "RenderableInterface.h"

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

	void render() override
	{
		this->model->Draw(this);
	}
private:

};


class Object 
{
public:
	Object(Orientation orientation, Renderable* renderer) : orientation{ orientation }, renderer{renderer}
	{
		
		assert(renderer && "Object class failed to instantiate. Renderer is a nullptr");
		renderer->orientation = &this->orientation;
	}

public:
	Orientation orientation;
	Renderable* renderer;

};