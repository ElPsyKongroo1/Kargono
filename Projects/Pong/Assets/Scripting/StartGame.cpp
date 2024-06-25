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

	//Random rnd = new Random();
	//Int32 directionChoice = rnd.Next(2);
	//Vector2 velocity = Vector2.Zero;

	//if (directionChoice == 0)
	//{
	//	velocity.X = -1.0f;
	//}
	//else
	//{
	//	velocity.X = 1.0f;
	//}
	//ball.m_Rigidbody.LinearVelocity = velocity * ball.Speed;
}









