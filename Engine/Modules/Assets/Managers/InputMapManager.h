#pragma once

#include "Modules/Assets/AssetManager.h"
#include "Modules/InputMap/Assets/InputMap.h"

namespace Kargono::Assets
{
	static inline AssetManager<InputMap::InputMap> s_InputMapManager{};
}