void KG_FUNC_14683932765512752045()
{
	LoadUserInterfaceFromName("UserInterface/Main Menu.kgui");
	TransitionSceneFromName("Scenes/main_menu.kgscene");
	LoadInputModeByName("Input/MainMenu.kginput");
	PlayStereoSoundFromName("Audio/Manoria-Cathedral.wav");
	RequestUserCount();
}
