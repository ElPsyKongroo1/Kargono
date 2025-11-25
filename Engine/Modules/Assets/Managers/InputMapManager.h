#pragma once

#include "Modules/Assets/AssetManager.h"
#include "Modules/InputMap/Assets/InputMap.h"

namespace Kargono::Assets
{
	extern AssetManager<InputMap::InputMap> s_InputMapManager;
}