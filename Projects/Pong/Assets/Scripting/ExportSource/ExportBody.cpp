#include "ExportHeader.h"
#include <iostream>
namespace Kargono
{
static std::function<void(const std::string& a)> PlaySoundFromNamePtr {};
void PlaySoundFromName(const std::string& a)
{
	PlaySoundFromNamePtr(a);
}
void AddVoidNone(const std::string& funcName, std::function<void()> funcPtr)
{
}
void AddVoidString(const std::string& funcName, std::function<void(const std::string&)> funcPtr)
{
if (funcName == "PlaySoundFromName") { PlaySoundFromNamePtr = funcPtr; return; }
}
void KG_FUNC_753079771072191180()
{
}

void KG_FUNC_7448269885476125662()
{
	std::cout << "Hello world! HAHAHAHAA\n";
	PlaySoundFromName("Audio/menu_select.wav");
}

}
