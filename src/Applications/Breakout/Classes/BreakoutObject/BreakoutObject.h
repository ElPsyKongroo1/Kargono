#pragma once
#include "../../../Library/Application/GameApplication/GameObject/GameObject.h"

class GameBrick : public GameObject
{

public:
	GameBrick(Orientation orientation, ShapeRenderer* renderer, glm::ivec2& mapLocation, float baseSpeed, AudioSource* audioSource) : GameObject(orientation, renderer, baseSpeed, glm::normalize(glm::vec3(0.0f, 0.0f, 0.0f)), audioSource), mapLocation{ mapLocation }
	{
		float Dimension_X = meshDimensions.x * orientation.scale.x;
		float Dimension_Y = meshDimensions.y * orientation.scale.y;
		currentDimensions = glm::vec2(Dimension_X, Dimension_Y);

	}
public:
	glm::vec2 currentDimensions;
	glm::ivec2 mapLocation;
public:
	void remove() override {}

};

class GamePaddle : public GameObject
{

public:
	GamePaddle(Orientation orientation, ShapeRenderer* renderer, float baseSpeed, AudioSource* audioSource) : GameObject(orientation, renderer, baseSpeed, glm::vec3(0.0f, 1.0f, 0.0f), audioSource)
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
	float radius;
	bool Stuck;
public:
	GameBall(Orientation orientation, ShapeRenderer* renderer, float baseSpeed, AudioSource* audioSource) : GameObject(orientation, renderer, baseSpeed, glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)), audioSource),
		radius{ renderer->mesh->dimensions.x }, Stuck{ true }
	{
		radius = glm::max(meshDimensions.x * orientation.scale.x, meshDimensions.y * orientation.scale.y) / 2;
		Stuck = false;

	}
public:
	void Move();
	void remove() override {}

};