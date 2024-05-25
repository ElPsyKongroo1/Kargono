void ApproveJoinSession(uint16_t userSlot)
{
	uint16_t direction = 0;
	SetGameStateField("BallDirection", &direction);
	LoadUserInterfaceFromName("UserInterface/RuntimeUI.kgui");
	SetDisplayWindow("online_lobby", true);
	SetDisplayWindow("base_window", false);
	TransitionSceneFromName("Scenes/main_gameplay.kgscene");
	LoadInputModeByName("Input/Online_Lobby_Input.kginput");
	PlaySoundFromName("Audio/menu_confirm.wav");
	PlayStereoSoundFromName("Audio/greenA.wav");
	
	std::string selectedWidget = std::string("player_slot_") + std::to_string(userSlot);
	SetWidgetText("online_lobby", selectedWidget, "Connected!");
}