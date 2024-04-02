void KG_FUNC_1819971799574496652()
{
		uint16_t userSlot = GetActiveSessionSlot();
		SetDisplayWindow("pre_game_warning", true);
		SetDisplayWindow("base_window", false);
		SetDisplayWindow("online_lobby", false);
		LoadInputModeByName("Input/Online_Pre_Start.kginput");
		PlaySoundFromName("Audio/menu_confirm.wav");
		if (userSlot == 0)
		{
			SetWidgetText("pre_game_warning", "controls_2", "");
		}
		else if (userSlot == 1)
		{
			SetWidgetText("pre_game_warning", "controls_1", "");
		}
		EnableReadyCheck();
}
