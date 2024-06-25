void Player2MoveDown(uint64_t entity,float timeStep)
{
	Math::vec3 translation = TransformComponent_GetTranslation(entity);
	bool upperLimit = translation.y >= 11.15;
	bool lowerLimit = translation.y <= -11.15;

	//float speed = Input.IsKeyDown(KeyCode.RightShift) ? Speed * SpeedUpFactor : Speed;
	//Vector3 velocity = Vector3.Zero;

	//if (!lowerLimit) { velocity.Y = -1.0f; }

	//velocity *= speed * ts;
	//Translation += velocity;
}


