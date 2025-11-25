#pragma once

#include "Modules/Assets/AssetManager.h"
#include "Modules/States/Assets/State.h"

namespace Kargono::Assets
{
	extern AssetManager<States::State> s_StateManager;
}