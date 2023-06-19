#pragma once
#include "../../../Library/Includes.h"
#include "../../../Library/Application/Objects/Objects.h"
class GameObject : public Object
{
public:
	float objectSpeed;
	glm::vec3 direction;
protected:
	glm::vec2 meshDimensions;
public:
	GameObject(Orientation orientation, Renderable* renderer) : Object(orientation, renderer), objectSpeed{ 1.0f }
	{
		const float half_to_full_length_conversion = 2;
		direction = glm::normalize(glm::vec3(2.0f, 1.0f, 0.0f));
		float Dimension_X = static_cast<ShapeRenderer*>(renderer)->mesh->dimensions.x * half_to_full_length_conversion;
		float Dimension_Y = static_cast<ShapeRenderer*>(renderer)->mesh->dimensions.y * half_to_full_length_conversion;
		meshDimensions = glm::vec2(Dimension_X, Dimension_Y);
	}
public:
	virtual void remove() = 0;

};

class GameBrick : public GameObject
{

public:
	GameBrick(Orientation orientation, ShapeRenderer* renderer, glm::ivec2& mapLocation) : GameObject(orientation, renderer), mapLocation{ mapLocation }
	{
		float Dimension_X = meshDimensions.x * orientation.scale.x;
		float Dimension_Y = meshDimensions.y * orientation.scale.y;
		currentDimensions = glm::vec2(Dimension_X, Dimension_Y);

	}
public:
	glm::vec2 currentDimensions;
	glm::ivec2 mapLocation;
public:
	void remove() override;

};

class GamePaddle : public GameObject
{

public:
	GamePaddle(Orientation orientation, ShapeRenderer* renderer) : GameObject(orientation, renderer)
	{
		float Dimension_X = meshDimensions.x * orientation.scale.x;
		float Dimension_Y = meshDimensions.y * orientation.scale.y;
		currentDimensions = glm::vec2(Dimension_X, Dimension_Y);
	}
public:
	glm::vec2 currentDimensions;
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
		Radius = glm::max(meshDimensions.x * orientation.scale.x, meshDimensions.y * orientation.scale.y) / 2;
		Stuck = false;

	}
public:
	void Move();
	void remove() override {}

};