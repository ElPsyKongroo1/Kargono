#include "ExportHeader.h"
#include <iostream>
namespace Kargono
{
static std::function<void(const std::string& a)> PlaySoundFromNamePtr {};
void PlaySoundFromName(const std::string& a)
{
	PlaySoundFromNamePtr(a);
}
static std::function<void(const std::string& a, const std::string& b, const std::string& c)> SetWidgetTextPtr {};
void SetWidgetText(const std::string& a, const std::string& b, const std::string& c)
{
	SetWidgetTextPtr(a, b, c);
}
void AddVoidNone(const std::string& funcName, std::function<void()> funcPtr)
{
}
void AddVoidString(const std::string& funcName, std::function<void(const std::string&)> funcPtr)
{
if (funcName == "PlaySoundFromName") { PlaySoundFromNamePtr = funcPtr; return; }
}
void AddVoidStringStringString(const std::string& funcName, std::function<void(const std::string&, const std::string&, const std::string&)> funcPtr)
{
if (funcName == "SetWidgetText") { SetWidgetTextPtr = funcPtr; return; }
}
void KG_FUNC_7448269885476125662()
{
	std::cout << "Bruh IDK what the hell is going on?\n";
	PlaySoundFromName("Audio/menu_select.wav");
}

void KG_FUNC_3176244785148247992(uint16_t userSlot)
{
	std::string selectedWidget = "player_slot_" + std::to_string(userSlot);
	SetWidgetText("online_lobby", selectedWidget, "Connected!");
}

void KG_FUNC_7445822592925037095(uint16_t userSlot)
{
	std::string selectedWidget = "player_slot_" + std::to_string(userSlot);
	std::string newText = "No Player " + std::to_string(userSlot + 1);

	SetWidgetText("online_lobby", selectedWidget, newText);

	SetWidgetText("online_lobby", "main_text", "Waiting for Players...");
}

}
