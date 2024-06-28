void OnReceiveSignal(uint16_t signalNum)
{
	uint16_t userSlot = GetActiveSessionSlot();
	
	if (signalNum == 1)
	{
		uint16_t ball = FindEntityHandleByName("Ball");
		if (userSlot == 0)
		{
			SetEntityFieldByName(ball, "PlayerOneScore", (void*)&RValueToLValue(*(uint32_t*)GetEntityFieldByName(ball, "PlayerOneScore") + 1));
			SetWidgetText("base_window", "score_player_1", std::to_string(*(uint32_t*)GetEntityFieldByName(ball, "PlayerOneScore")));
		}
		else if (userSlot == 1)
		{
			SetEntityFieldByName(ball, "PlayerTwoScore", (void*)&RValueToLValue(*(uint32_t*)GetEntityFieldByName(ball, "PlayerTwoScore") + 1));
			SetWidgetText("base_window", "score_player_2", std::to_string(*(uint32_t*)GetEntityFieldByName(ball, "PlayerTwoScore")));
		}
		Rigidbody2DComponent_SetLinearVelocity(ball, {0.0f, 0.0f});
		PlaySoundFromName("Audio/lose_sound.wav");
		SetDisplayWindow("pre_game_warning", true);
		LoadInputModeByName("Input/Online_Pre_Start.kginput");
		EnableReadyCheck();
	}
	
}




