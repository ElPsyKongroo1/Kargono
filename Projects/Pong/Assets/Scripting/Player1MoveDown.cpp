void Player1MoveDown(uint64_t entity,float timeStep)
{
	Math::vec3 translation = TransformComponent_GetTranslation(entity);
	bool upperLimit = translation.y >= 11.15;
	bool lowerLimit = translation.y <= -11.15;

	float speed = IsKeyPressed(Key::LeftShift) ? *(float*)GetEntityFieldByName(entity, "Speed") * *(float*)GetEntityFieldByName(entity, "SpeedUpFactor") : *(float*)GetEntityFieldByName(entity, "Speed");
	Math::vec3 velocity = {0.0f, 0.0f, 0.0f};

	if (!lowerLimit) 
	{
		velocity.y = -1.0f; 
	}

	velocity *= speed * timeStep;
	TransformComponent_SetTranslation(entity, TransformComponent_GetTranslation(entity) + velocity);
}


