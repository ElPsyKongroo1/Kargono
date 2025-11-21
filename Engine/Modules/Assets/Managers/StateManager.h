#pragma once

#include "Modules/Assets/AssetManager.h"
#include "Modules/States/Assets/State.h"

namespace Kargono::Assets
{
	static inline AssetManager<States::State> s_StateManager{};
}