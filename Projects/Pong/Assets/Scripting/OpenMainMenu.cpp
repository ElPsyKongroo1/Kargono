void OpenMainMenu()
{
	LoadUserInterfaceFromName("UserInterface/Main Menu.kgui");
	TransitionSceneFromName("Scenes/main_menu.kgscene");
	LoadInputModeByName("Input/MainMenu.kginput");
	PlayStereoSoundFromName("Audio/blueA.wav");
	RequestUserCount();
}