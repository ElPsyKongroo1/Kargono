#pragma once
#include "../../../Library/Includes.h"
#include "../../../Library/Application/Objects/Objects.h"
class GameObject : public Object
{
public:
	float objectSpeed;
	glm::vec3 direction;
protected:
	glm::vec3 objectCurrentDimensions;
public:
	GameObject(Orientation orientation, Renderable* renderer) : Object(orientation, renderer), objectSpeed{ 1.0f }
	{
		direction = glm::normalize(glm::vec3(2.0f, 1.0f, 0.0f));
	}
public:
	virtual void remove() = 0;

};

class GameBrick : public GameObject
{

public:
	GameBrick(Orientation orientation, ShapeRenderer* renderer, glm::ivec2& mapLocation) : GameObject(orientation, renderer), mapLocation{ mapLocation }
	{

	}
public:
	glm::ivec2 mapLocation;
public:
	void remove() override;

};

class GamePaddle : public GameObject
{

public:
	GamePaddle(Orientation orientation, ShapeRenderer* renderer) : GameObject(orientation, renderer)
	{

	}
public:
	void remove() override {}

};

class GameBall : public GameObject
{
public:
	float Radius;
	bool Stuck;
public:
	GameBall(Orientation orientation, ShapeRenderer* renderer) : GameObject(orientation, renderer), 
		Radius{ renderer->mesh->dimensions.x }, Stuck{true}
	{

	}
public:
	void Move();
	void remove() override {}

};