#pragma once

#include "Kargono/Core/FixedString.h"

#include <vector>
#include <cstdint>

namespace Kargono::ProjectData
{
	using HexCode = uint32_t;

	struct Color
	{
		FixedString32 m_Name;
		HexCode m_HexCode;
	};

	struct ColorPalette
	{
		std::vector<Color> m_Colors;
	};

	class ColorPaletteEditor
	{

	};
}
