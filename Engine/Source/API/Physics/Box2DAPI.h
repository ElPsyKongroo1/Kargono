#pragma once

#include "box2d/b2_body.h"
#include "box2d/b2_world.h"

namespace Kargono::Utility
{
	//==============================
	// Simple Conversions
	//==============================

	inline static b2BodyType Rigidbody2DTypeToBox2DBody(ECS::Rigidbody2DComponent::BodyType bodyType)
	{
		switch (bodyType)
		{
		case ECS::Rigidbody2DComponent::BodyType::Static:	return b2_staticBody;
		case ECS::Rigidbody2DComponent::BodyType::Dynamic:	return b2_dynamicBody;
		case ECS::Rigidbody2DComponent::BodyType::Kinematic:	return b2_kinematicBody;
		}
		KG_ERROR("Unknown body type");
		return b2_staticBody;
	}

	inline static ECS::Rigidbody2DComponent::BodyType Rigidbody2DTypeFromBox2DBody(b2BodyType bodyType)
	{
		switch (bodyType)
		{
		case b2_staticBody:		return ECS::Rigidbody2DComponent::BodyType::Static;
		case b2_dynamicBody:	return ECS::Rigidbody2DComponent::BodyType::Dynamic;
		case b2_kinematicBody:	return ECS::Rigidbody2DComponent::BodyType::Kinematic;
		}
		KG_ERROR("Unknown body type");
		return ECS::Rigidbody2DComponent::BodyType::Static;
	}
}
