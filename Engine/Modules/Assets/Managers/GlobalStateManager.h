#pragma once

#include "Modules/Assets/AssetManager.h"
#include "Modules/GlobalState/Assets/GlobalState.h"

namespace Kargono::Assets
{
	static inline AssetManager<GlobalState::GlobalState> s_GlobalStateManager{};
}