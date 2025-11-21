#pragma once

#include "Modules/Assets/AssetManager.h"
#include "Modules/Scenes/Assets/Scene.h"

namespace Kargono::Assets
{
	static inline AssetManager<Scenes::Scene> s_SceneManager{};
}