#include "ExportHeader.h"
#include <iostream>
namespace Kargono
{
static std::function<void()> EnableReadyCheckPtr {};
void EnableReadyCheck()
{
	EnableReadyCheckPtr();
}
static std::function<void()> RequestUserCountPtr {};
void RequestUserCount()
{
	RequestUserCountPtr();
}
static std::function<uint16_t()> GetActiveSessionSlotPtr {};
uint16_t GetActiveSessionSlot()
{
	return GetActiveSessionSlotPtr();
}
static std::function<void(const std::string& a)> PlaySoundFromNamePtr {};
void PlaySoundFromName(const std::string& a)
{
	PlaySoundFromNamePtr(a);
}
static std::function<void(const std::string& a)> PlayStereoSoundFromNamePtr {};
void PlayStereoSoundFromName(const std::string& a)
{
	PlayStereoSoundFromNamePtr(a);
}
static std::function<void(const std::string& a)> LoadInputModeByNamePtr {};
void LoadInputModeByName(const std::string& a)
{
	LoadInputModeByNamePtr(a);
}
static std::function<void(const std::string& a)> LoadUserInterfaceFromNamePtr {};
void LoadUserInterfaceFromName(const std::string& a)
{
	LoadUserInterfaceFromNamePtr(a);
}
static std::function<void(const std::string& a)> TransitionSceneFromNamePtr {};
void TransitionSceneFromName(const std::string& a)
{
	TransitionSceneFromNamePtr(a);
}
static std::function<void(const std::string& a, bool b)> SetDisplayWindowPtr {};
void SetDisplayWindow(const std::string& a, bool b)
{
	SetDisplayWindowPtr(a, b);
}
static std::function<void(const std::string& a, const std::string& b)> SetSelectedWidgetPtr {};
void SetSelectedWidget(const std::string& a, const std::string& b)
{
	SetSelectedWidgetPtr(a, b);
}
static std::function<void(const std::string& a, void* b)> SetGameStateFieldPtr {};
void SetGameStateField(const std::string& a, void* b)
{
	SetGameStateFieldPtr(a, b);
}
static std::function<void(const std::string& a, const std::string& b, bool c)> SetWidgetSelectablePtr {};
void SetWidgetSelectable(const std::string& a, const std::string& b, bool c)
{
	SetWidgetSelectablePtr(a, b, c);
}
static std::function<void(const std::string& a, const std::string& b, const std::string& c)> SetWidgetTextPtr {};
void SetWidgetText(const std::string& a, const std::string& b, const std::string& c)
{
	SetWidgetTextPtr(a, b, c);
}
static std::function<void(const std::string& a, const std::string& b, Math::vec4 c)> SetWidgetTextColorPtr {};
void SetWidgetTextColor(const std::string& a, const std::string& b, Math::vec4 c)
{
	SetWidgetTextColorPtr(a, b, c);
}
static std::function<void(const std::string& a, const std::string& b, Math::vec4 c)> SetWidgetBackgroundColorPtr {};
void SetWidgetBackgroundColor(const std::string& a, const std::string& b, Math::vec4 c)
{
	SetWidgetBackgroundColorPtr(a, b, c);
}
void AddVoidNone(const std::string& funcName, std::function<void()> funcPtr)
{
if (funcName == "EnableReadyCheck") { EnableReadyCheckPtr = funcPtr; return; }
if (funcName == "RequestUserCount") { RequestUserCountPtr = funcPtr; return; }
}
void AddVoidString(const std::string& funcName, std::function<void(const std::string&)> funcPtr)
{
if (funcName == "PlaySoundFromName") { PlaySoundFromNamePtr = funcPtr; return; }
if (funcName == "PlayStereoSoundFromName") { PlayStereoSoundFromNamePtr = funcPtr; return; }
if (funcName == "LoadInputModeByName") { LoadInputModeByNamePtr = funcPtr; return; }
if (funcName == "TransitionSceneFromName") { TransitionSceneFromNamePtr = funcPtr; return; }
if (funcName == "LoadUserInterfaceFromName") { LoadUserInterfaceFromNamePtr = funcPtr; return; }
}
void AddVoidStringBool(const std::string& funcName, std::function<void(const std::string&, bool)> funcPtr)
{
if (funcName == "SetDisplayWindow") { SetDisplayWindowPtr = funcPtr; return; }
}
void AddVoidStringVoidPtr(const std::string& funcName, std::function<void(const std::string&, void*)> funcPtr)
{
if (funcName == "SetGameStateField") { SetGameStateFieldPtr = funcPtr; return; }
}
void AddVoidStringString(const std::string& funcName, std::function<void(const std::string&, const std::string&)> funcPtr)
{
if (funcName == "SetSelectedWidget") { SetSelectedWidgetPtr = funcPtr; return; }
}
void AddVoidStringStringBool(const std::string& funcName, std::function<void(const std::string&, const std::string&, bool)> funcPtr)
{
if (funcName == "SetWidgetSelectable") { SetWidgetSelectablePtr = funcPtr; return; }
}
void AddVoidStringStringString(const std::string& funcName, std::function<void(const std::string&, const std::string&, const std::string&)> funcPtr)
{
if (funcName == "SetWidgetText") { SetWidgetTextPtr = funcPtr; return; }
}
void AddVoidStringStringVec4(const std::string& funcName, std::function<void(const std::string&, const std::string&, Math::vec4)> funcPtr)
{
if (funcName == "SetWidgetTextColor") { SetWidgetTextColorPtr = funcPtr; return; }
if (funcName == "SetWidgetBackgroundColor") { SetWidgetBackgroundColorPtr = funcPtr; return; }
}
void AddUInt16None(const std::string& funcName, std::function<uint16_t()> funcPtr)
{
if (funcName == "GetActiveSessionSlot") { GetActiveSessionSlotPtr = funcPtr; return; }
}
void UpdateSessionUserSlot(uint16_t userSlot)
{
	std::string selectedWidget = "player_slot_" + std::to_string(userSlot);
	SetWidgetText("online_lobby", selectedWidget, "Connected!");
}

void OnCurrentSessionInit()
{
	SetWidgetText("online_lobby", "main_text", "Starting Session...");
}

void UserLeftSession(uint16_t userSlot)
{
	std::string selectedWidget = "player_slot_" + std::to_string(userSlot);
	std::string newText = "No Player " + std::to_string(userSlot + 1);

	SetWidgetText("online_lobby", selectedWidget, newText);

	SetWidgetText("online_lobby", "main_text", "Waiting for Players...");
}

void UpdateOnlineCount(uint32_t count)
{
	std::string onlineCount = std::string("Online: ") + std::to_string(count);
	SetWidgetText("main_window", "online_count", onlineCount);
	SetWidgetSelectable("main_window", "online_multiplayer", true);
	SetWidgetTextColor("main_window", "online_multiplayer", Math::vec4(1.0f));
	SetWidgetBackgroundColor("main_window", "online_multiplayer", {103.0f / 255.0f, 17.0f / 255.0f, 175.0f / 255.0f, 54.0f / 255.0f});
}
void OnStartSession()
{	
		uint16_t userSlot = GetActiveSessionSlot();
		SetDisplayWindow("pre_game_warning", true);
		SetDisplayWindow("base_window", false);
		SetDisplayWindow("online_lobby", false);
		LoadInputModeByName("Input/Online_Pre_Start.kginput");
		PlaySoundFromName("Audio/menu_confirm.wav");
		if (userSlot == 0)
		{
			SetWidgetText("pre_game_warning", "controls_2", "");
		}
		else if (userSlot == 1)
		{
			SetWidgetText("pre_game_warning", "controls_1", "");
		}
		EnableReadyCheck();
}

void ApproveJoinSession(uint16_t userSlot)
{
	uint16_t direction = 0;
	SetGameStateField("BallDirection", &direction);
	LoadUserInterfaceFromName("UserInterface/RuntimeUI.kgui");
	SetDisplayWindow("online_lobby", true);
	SetDisplayWindow("base_window", false);
	TransitionSceneFromName("Scenes/main_gameplay.kgscene");
	LoadInputModeByName("Input/Online_Lobby_Input.kginput");
	PlaySoundFromName("Audio/menu_confirm.wav");
	PlayStereoSoundFromName("Audio/mechanist-theme.wav");
	
	std::string selectedWidget = std::string("player_slot_") + std::to_string(userSlot);
	SetWidgetText("online_lobby", selectedWidget, "Connected!");
}
void OpenMainMenu()
{
	LoadUserInterfaceFromName("UserInterface/Main Menu.kgui");
	TransitionSceneFromName("Scenes/main_menu.kgscene");
	LoadInputModeByName("Input/MainMenu.kginput");
	PlayStereoSoundFromName("Audio/Manoria-Cathedral.wav");
	RequestUserCount();
}

void OnConnectionTerminated()
{
	SetWidgetText("main_window", "online_count", "Offline");
	SetWidgetSelectable("main_window", "online_multiplayer", false);
	SetWidgetTextColor("main_window", "online_multiplayer", {1.0f, 1.0f, 1.0f, 222.0f / 255.0f});
	SetWidgetBackgroundColor("main_window", "online_multiplayer", {30.0f / 255.0f, 30.0f / 255.0f, 30.0f / 255.0f, 37.0f / 255.0f});
	SetSelectedWidget("main_window", "local_multiplayer");
}

}
