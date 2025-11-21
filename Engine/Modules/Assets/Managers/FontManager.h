#pragma once

#include "Modules/Assets/AssetManager.h"
#include "Modules/RuntimeUI/Assets/Font.h"

namespace Kargono::Assets
{
	static inline AssetManager<RuntimeUI::Font> s_FontManager{};
}