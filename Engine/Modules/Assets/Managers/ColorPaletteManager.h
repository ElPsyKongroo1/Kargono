#pragma once

#include "Modules/Assets/AssetManager.h"
#include "Modules/EditorUI/Assets/ColorPalette.h"

namespace Kargono::Assets
{
	static inline AssetManager<EditorUI::ColorPalette> s_ColorPaletteManager{};
}