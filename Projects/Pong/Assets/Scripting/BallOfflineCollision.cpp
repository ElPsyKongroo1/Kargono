bool BallOfflineCollision(uint64_t entity,uint64_t otherEntity)
{
	bool collisionHandled = false;
	std::string otherEntityTag = TagComponent_GetTag(otherEntity);
	
	if (otherEntityTag == "Left Wall")
	{
		SetEntityFieldByName(entity, "PlayerTwoScore", (void*)&RValueToLValue(*(uint32_t*)GetEntityFieldByName(entity, "PlayerTwoScore") + 1));
		SetWidgetText("base_window", "score_player_2", std::to_string(*(uint32_t*)GetEntityFieldByName(entity, "PlayerTwoScore")));
		Rigidbody2DComponent_SetLinearVelocity(entity, {0.0f, 0.0f});
		PlaySoundFromName("Audio/lose_sound.wav");
		SetDisplayWindow("pre_game_warning", true);
		LoadInputModeByName("Input/Pre_Start.kginput");
		collisionHandled = true;
	}
	else if (otherEntityTag == "Right Wall")
	{
		SetEntityFieldByName(entity, "PlayerOneScore", (void*)&RValueToLValue(*(uint32_t*)GetEntityFieldByName(entity, "PlayerOneScore") + 1));
		SetWidgetText("base_window", "score_player_1", std::to_string(*(uint32_t*)GetEntityFieldByName(entity, "PlayerOneScore")));
		Rigidbody2DComponent_SetLinearVelocity(entity, {0.0f, 0.0f});
		PlaySoundFromName("Audio/lose_sound.wav");
		SetDisplayWindow("pre_game_warning", true);
		LoadInputModeByName("Input/Pre_Start.kginput");
		collisionHandled = true;
	}
	else if (otherEntityTag == "Top Wall" || otherEntityTag == "Bottom Wall")
	{
		Math::vec2 horizontalDirection;
		Math::vec2 currentVelocity = Rigidbody2DComponent_GetLinearVelocity(entity);
		if (currentVelocity.x > 0)
		{
			horizontalDirection = {1.0f, 0.0f};
		}
		else
		{
			horizontalDirection =  {-1.0f, 0.0f};
		}
		currentVelocity = glm::normalize(glm::normalize(currentVelocity) + (horizontalDirection * 0.1f)) * *(float*)GetEntityFieldByName(entity, "Speed");
		Rigidbody2DComponent_SetLinearVelocity(entity, currentVelocity);
		PlaySoundFromName("Audio/pop-sound.wav");
		collisionHandled = true;
	}
	
	else if (otherEntityTag == "Player1")
	{
		if (IsKeyPressed(Key::W))
		{
			float deflectionFactor = IsKeyPressed(Key::LeftShift) ? 0.55f : 0.33f;
			Math::vec2 up = {0.0f, 1.0f};
			Math::vec2 currentVelocity = Rigidbody2DComponent_GetLinearVelocity(entity);
			currentVelocity = glm::normalize(glm::normalize(currentVelocity) + (up * deflectionFactor)) * *(float*)GetEntityFieldByName(entity, "Speed");
			Rigidbody2DComponent_SetLinearVelocity(entity, currentVelocity);
		}
		if (IsKeyPressed(Key::A))
		{
			float deflectionFactor = IsKeyPressed(Key::LeftShift) ? 0.55f : 0.33f;
			Math::vec2 up = {0.0f, -1.0f};
			Math::vec2 currentVelocity = Rigidbody2DComponent_GetLinearVelocity(entity);
			currentVelocity = glm::normalize(glm::normalize(currentVelocity) + (up * deflectionFactor)) * *(float*)GetEntityFieldByName(entity, "Speed");
			Rigidbody2DComponent_SetLinearVelocity(entity, currentVelocity);
		}
		PlaySoundFromName("Audio/pop-sound.wav");
		collisionHandled = true;
	}
	else if (otherEntityTag == "Player2")
	{
		if (IsKeyPressed(Key::O))
		{
			float deflectionFactor = IsKeyPressed(Key::RightShift) ? 0.55f : 0.33f;
			Math::vec2 up = {0.0f, 1.0f};
			Math::vec2 currentVelocity = Rigidbody2DComponent_GetLinearVelocity(entity);
			currentVelocity = glm::normalize(glm::normalize(currentVelocity) + (up * deflectionFactor)) * *(float*)GetEntityFieldByName(entity, "Speed");
			Rigidbody2DComponent_SetLinearVelocity(entity, currentVelocity);
		}
		if (IsKeyPressed(Key::Semicolon))
		{
			float deflectionFactor = IsKeyPressed(Key::RightShift) ? 0.55f : 0.33f;
			Math::vec2 up = {0.0f, -1.0f};
			Math::vec2 currentVelocity = Rigidbody2DComponent_GetLinearVelocity(entity);
			currentVelocity = glm::normalize(glm::normalize(currentVelocity) + (up * deflectionFactor)) * *(float*)GetEntityFieldByName(entity, "Speed");
			Rigidbody2DComponent_SetLinearVelocity(entity, currentVelocity);
		}
		PlaySoundFromName("Audio/pop-sound.wav");
		collisionHandled = true;
	}
	
	return collisionHandled;
}
