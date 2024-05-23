void UpdateSessionUserSlot(uint16_t userSlot)
{
	std::string selectedWidget = "player_slot_" + std::to_string(userSlot);
	SetWidgetText("online_lobby", selectedWidget, "Connected!");
}
