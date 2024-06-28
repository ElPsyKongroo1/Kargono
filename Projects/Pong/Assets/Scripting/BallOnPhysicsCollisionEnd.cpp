bool BallOnPhysicsCollisionEnd(uint64_t entity,uint64_t otherEntity)
{
	uint16_t userSlot = GetActiveSessionSlot();
	if (userSlot == std::numeric_limits<uint16_t>().max())
	{
		return false;
	}

	bool collisionHandled = false;
	std::string otherEntityTag = TagComponent_GetTag(otherEntity);
	
	if (otherEntityTag == "Player1")
	{
		if (userSlot == 0)
		{
			Math::vec3 updateVector = TransformComponent_GetTranslation(entity);
			updateVector.x += 0.2f;
			SendAllEntityPhysics(entity, updateVector, Rigidbody2DComponent_GetLinearVelocity(entity));
		}
		collisionHandled = true;
	}
	else if (otherEntityTag == "Player2")
	{
		if (userSlot == 1)
		{
			Math::vec3 updateVector = TransformComponent_GetTranslation(entity);
			updateVector.x += -0.2f;
			SendAllEntityPhysics(entity, updateVector, Rigidbody2DComponent_GetLinearVelocity(entity));
		}
		collisionHandled = true;
	}
	return collisionHandled;
}






