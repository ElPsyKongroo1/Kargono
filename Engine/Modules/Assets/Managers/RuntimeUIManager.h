#pragma once

#include "Modules/Assets/AssetManager.h"
#include "Modules/RuntimeUI/Assets/RuntimeUIUserInterface.h"

namespace Kargono::Assets
{
	static inline AssetManager<RuntimeUI::UserInterface> s_UserInterfaceManager{};
}