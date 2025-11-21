#pragma once

#include "Modules/Assets/AssetManager.h"
#include "Modules/Rendering/Assets/Texture2D.h"

namespace Kargono::Assets
{
	static inline AssetManager<Rendering::Texture2D> s_Texture2DManager{};
}