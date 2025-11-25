#pragma once

#include "Modules/Assets/AssetManager.h"
#include "Modules/GlobalState/Assets/GameState.h"

namespace Kargono::Assets
{
	extern AssetManager<GlobalState::GameState> s_GameStateManager;
}