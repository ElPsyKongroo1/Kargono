void OpenPongGameplay()
{
	LoadUserInterfaceFromName("UserInterface/RuntimeUI.kgui");
	SetDisplayWindow("pre_game_warning", true);
	SetDisplayWindow("base_window", false);
	TransitionSceneFromName("Scenes/main_gameplay.kgscene");
	LoadInputModeByName("Input/Pre_Start.kginput");
	PlaySoundFromName("Audio/menu_confirm.wav");
	PlayStereoSoundFromName("Audio/greenA.wav");
}
