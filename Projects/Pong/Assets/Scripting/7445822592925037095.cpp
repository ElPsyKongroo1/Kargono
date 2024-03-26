void KG_FUNC_7445822592925037095(uint16_t userSlot)
{
	std::string selectedWidget = "player_slot_" + std::to_string(userSlot);
	std::string newText = "No Player " + std::to_string(userSlot + 1);

	SetWidgetText("online_lobby", selectedWidget, newText);

	SetWidgetText("online_lobby", "main_text", "Waiting for Players...");
}
