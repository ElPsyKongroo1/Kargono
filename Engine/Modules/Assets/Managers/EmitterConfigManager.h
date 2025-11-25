#pragma once

#include "Modules/Assets/AssetManager.h"
#include "Modules/Particles/Assets/EmitterConfig.h"

namespace Kargono::Assets
{
	extern AssetManager<Particles::EmitterConfig> s_EmitterConfigManager;
}