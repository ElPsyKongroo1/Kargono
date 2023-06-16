#pragma once
#include "../../../Library/Includes.h"
#include "../../../Library/Application/Objects/Objects.h"
class GameObject : public Object
{
public:
	float objectVelocity;
public:
	GameObject(Orientation orientation, Renderable* renderer) : Object(orientation, renderer), objectVelocity{1.0f}
	{

	}
};