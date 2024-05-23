void OnConnectionTerminated()
{
	SetWidgetText("main_window", "online_count", "Offline");
	SetWidgetSelectable("main_window", "online_multiplayer", false);
	SetWidgetTextColor("main_window", "online_multiplayer", {1.0f, 1.0f, 1.0f, 222.0f / 255.0f});
	SetWidgetBackgroundColor("main_window", "online_multiplayer", {30.0f / 255.0f, 30.0f / 255.0f, 30.0f / 255.0f, 37.0f / 255.0f});
	SetSelectedWidget("main_window", "local_multiplayer");
}
