void StartGame()
{
	LoadInputModeByName("Input/Runtime.kginput");
	SetDisplayWindow("pre_game_warning", false);
	SetDisplayWindow("online_lobby", false);
	SetDisplayWindow("base_window", true);
	uint64_t player1 = FindEntityHandleByName("Player1");
	Log(std::to_string(player1));
	//player1.m_Transform.Translation = player1.InitialPosition;
	
	uint64_t player2 = FindEntityHandleByName("Player2");
	Log(std::to_string(player2));
	//player2.m_Transform.Translation = player2.InitialPosition;
	
	uint64_t ball = FindEntityHandleByName("Ball");
	Log(std::to_string(ball));
	//ball.m_Transform.Translation = ball.InitialPosition;

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




