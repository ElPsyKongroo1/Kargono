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
	std::cout << "Hello world! HAHAHAHAA\n";
	std::cout << "AHHHKJASHDKHASDIUHASDIUSAH\n";
	PlaySoundFromName("Audio/menu_select.wav");
}

void KG_FUNC_3176244785148247992(uint16_t userSlot)
{
	std::string selectedWidget = "player_slot_" + std::to_string(userSlot);
	SetWidgetText("online_lobby", selectedWidget, "Connected!");
}

}
