#pragma once
#include "../../../Library/Includes.h"
#include "../../../Library/Application/Objects/Objects.h"
class GameObject : public Object
{
public:
	float objectSpeed;
	glm::vec3 direction;
private:
	glm::vec3 objectCurrentDimensions;
public:
	GameObject(Orientation orientation, Renderable* renderer) : Object(orientation, renderer), objectSpeed{ 1.0f }
	{
		direction = glm::normalize(glm::vec3(2.0f, 1.0f, 0.0f));
	}
public:


};

class GameBall : public GameObject
{
public:
	float Radius;
	bool Stuck;
private:
	glm::vec3 objectCurrentDimensions;
public:
	GameBall(Orientation orientation, ShapeRenderer* renderer) : GameObject(orientation, renderer), 
		Radius{ renderer->mesh->dimensions.x }, Stuck{true}
	{

	}
public:
	void Move();

};