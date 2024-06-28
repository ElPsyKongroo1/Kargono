void StartOnlineGame()
{
	uint16_t userSlot = GetActiveSessionSlot();
	if (userSlot == 0)
	{
		LoadInputModeByName("Input/Online_Player1_Runtime.kginput");
	}
	else if (userSlot == 1)
	{
		LoadInputModeByName("Input/Online_Player2_Runtime.kginput");
	}
	
	SetDisplayWindow("pre_game_warning", false);
	SetDisplayWindow("online_lobby", false);
	SetDisplayWindow("base_window", true);
	uint64_t player1 = FindEntityHandleByName("Player1");
	TransformComponent_SetTranslation(player1, *(Math::vec3*)GetEntityFieldByName(player1, "InitialPosition"));
	
	uint64_t player2 = FindEntityHandleByName("Player2");
	TransformComponent_SetTranslation(player2, *(Math::vec3*)GetEntityFieldByName(player2, "InitialPosition"));
	
	uint64_t ball = FindEntityHandleByName("Ball");
	TransformComponent_SetTranslation(ball, *(Math::vec3*)GetEntityFieldByName(ball, "InitialPosition"));
	
	uint16_t ballDirection = *(uint16_t*)GetGameStateField("BallDirection");
	Math::vec2 velocity = {0.0f, 0.0f};
	if (ballDirection == 0)
	{
		velocity.x = -1.0f;
		SetGameStateField("BallDirection", &RValueToLValue(1));
	}
	else
	{
		velocity.x = 1.0f;
		SetGameStateField("BallDirection", &RValueToLValue(0));
	}
	
	Rigidbody2DComponent_SetLinearVelocity(ball, velocity * *(float*)GetEntityFieldByName(ball, "Speed"));
}





