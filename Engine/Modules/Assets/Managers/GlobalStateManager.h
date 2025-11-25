#pragma once

#include "Modules/Assets/AssetManager.h"
#include "Modules/GlobalState/Assets/GlobalState.h"

namespace Kargono::Assets
{
	extern AssetManager<GlobalState::GlobalState> s_GlobalStateManager;
}