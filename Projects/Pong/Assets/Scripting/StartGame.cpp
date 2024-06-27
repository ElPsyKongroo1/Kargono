void StartGame()
{
	LoadInputModeByName("Input/Runtime.kginput");
	SetDisplayWindow("pre_game_warning", false);
	SetDisplayWindow("online_lobby", false);
	SetDisplayWindow("base_window", true);
	uint64_t player1 = FindEntityHandleByName("Player1");
	TransformComponent_SetTranslation(player1, *(Math::vec3*)GetEntityFieldByName(player1, "InitialPosition"));
	
	uint64_t player2 = FindEntityHandleByName("Player2");
	TransformComponent_SetTranslation(player2, *(Math::vec3*)GetEntityFieldByName(player2, "InitialPosition"));
	
	uint64_t ball = FindEntityHandleByName("Ball");
	TransformComponent_SetTranslation(ball, *(Math::vec3*)GetEntityFieldByName(ball, "InitialPosition"));
	
	int32_t directionChoice = GenerateRandomNumber(0, 1);
	Math::vec2 velocity = {0.0f, 0.0f};

	if (directionChoice == 0)
	{
		velocity.x = -1.0f;
	}
	else
	{
		velocity.x = 1.0f;
	}
	
	Rigidbody2DComponent_SetLinearVelocity(ball, velocity * *(float*)GetEntityFieldByName(ball, "Speed"));
}












