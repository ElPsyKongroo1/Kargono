void KG_FUNC_3176244785148247992(uint16_t userSlot)
{
	std::string selectedWidget = "player_slot_" + std::to_string(userSlot);
	SetWidgetText("online_lobby", selectedWidget, "Connected!");
}
