#pragma once

#include "Modules/Physics2D/Components/RigidBody2D.h"

#include "box2d/b2_body.h"
#include "box2d/b2_world.h"

namespace Kargono::Utility
{
	//==============================
	// Simple Conversions
	//==============================

	inline b2BodyType RigidBody2DTypeToBox2DBody(Physics2D::BodyType bodyType)
	{
		switch (bodyType)
		{
		case Physics2D::BodyType::Static:	return b2_staticBody;
		case Physics2D::BodyType::Dynamic:	return b2_dynamicBody;
		case Physics2D::BodyType::Kinematic:	return b2_kinematicBody;
		}
		KG_ERROR("Unknown body type");
		return b2_staticBody;
	}

	inline Physics2D::BodyType RigidBody2DTypeFromBox2DBody(b2BodyType bodyType)
	{
		switch (bodyType)
		{
		case b2_staticBody:		return Physics2D::BodyType::Static;
		case b2_dynamicBody:	return Physics2D::BodyType::Dynamic;
		case b2_kinematicBody:	return Physics2D::BodyType::Kinematic;
		}
		KG_ERROR("Unknown body type");
		return Physics2D::BodyType::Static;
	}
}
