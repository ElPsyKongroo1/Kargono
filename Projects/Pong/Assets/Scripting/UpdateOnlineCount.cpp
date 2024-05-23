void UpdateOnlineCount(uint32_t count)
{
	std::string onlineCount = std::string("Online: ") + std::to_string(count);
	SetWidgetText("main_window", "online_count", onlineCount);
	SetWidgetSelectable("main_window", "online_multiplayer", true);
	SetWidgetTextColor("main_window", "online_multiplayer", Math::vec4(1.0f));
	SetWidgetBackgroundColor("main_window", "online_multiplayer", {103.0f / 255.0f, 17.0f / 255.0f, 175.0f / 255.0f, 54.0f / 255.0f});
}