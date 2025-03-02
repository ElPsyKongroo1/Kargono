#pragma once

#include "Kargono/Core/FixedString.h"
#include "Kargono/Math/MathAliases.h"

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

namespace Kargono::Utility
{
	inline Math::vec4 HexToRGBA(uint32_t hexCode)
	{
		return
		{
			// For anyone curious: 
			//	(hexCode >> ##) places the relevant two digits at the begin of the number ##23## -> ####23
			//  Note that one hexidecimal character (Ex: A, 1, or F) corresponds to a bitshift of  >> 4
			//	(hexCode & 0xFF) extracts the first two digits from the number (####23 -> 23)
			//	(num / 255.0f) normalizes the value between 0 - 1 (23 / 255.0f -> ~0.1f)
			((hexCode >> 24) & 0xFF) / 255.0f,	// Red
			((hexCode >> 16) & 0xFF) / 255.0f,	// Green
			((hexCode >> 8) & 0xFF) / 255.0f,	// Blue
			(hexCode & 0xFF) / 255.0f	// Alpha
		};
	}

	inline uint32_t RGBAToHex(const Math::vec4& rgba) 
	{
		return	((uint32_t)(rgba[0]	* 255.0f)	<< 24)	|	// Red
				((uint32_t)(rgba[1]	* 255.0f)	<< 16)	|	// Green
				((uint32_t)(rgba[2]	* 255.0f)	<< 8)	|	// Blue
				((uint32_t)(rgba[3]	* 255.0f));				// Alpha
	}
}
