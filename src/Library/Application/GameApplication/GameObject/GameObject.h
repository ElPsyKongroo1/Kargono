#pragma once
#include "../../../Includes.h"
#include "../../Objects/Objects.h"
class GameObject : public Object
{
public:
	float baseSpeed;
	float currentSpeed;
	glm::vec3 direction;
protected:
	glm::vec2 meshDimensions;
public:
	GameObject(Orientation orientation, Renderable* renderer, float baseSpeed, glm::vec3 direction, AudioSource* audioSource) : Object(orientation, renderer, audioSource), baseSpeed{ baseSpeed }, currentSpeed{ baseSpeed }, direction{direction}
	{
		 
		const float half_to_full_length_conversion = 2;
		float Dimension_X = static_cast<ShapeRenderer*>(renderer)->mesh->dimensions.x * half_to_full_length_conversion;
		float Dimension_Y = static_cast<ShapeRenderer*>(renderer)->mesh->dimensions.y * half_to_full_length_conversion;
		meshDimensions = glm::vec2(Dimension_X, Dimension_Y);
	}
public:
	virtual void remove() = 0;

};

class GameParticle : public GameObject
{

public:
	GameParticle(Orientation orientation, ShapeRenderer* renderer, float baseSpeed, float lifeTime, glm::vec3& direction, AudioSource* audioSource) : GameObject(orientation, renderer, baseSpeed, direction, audioSource), lifeTime{lifeTime}
	{
		float Dimension_X = meshDimensions.x * orientation.scale.x;
		float Dimension_Y = meshDimensions.y * orientation.scale.y;
		currentDimensions = glm::vec2(Dimension_X, Dimension_Y);

	}
public:
	float lifeTime;
	glm::vec2 currentDimensions;
	void remove() override {}
	void Move();

};

