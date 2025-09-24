#pragma once

#include "Kargono/Core/FixedBufferString.h"
#include "Kargono/Math/MathAliases.h"
#include "Modules/EditorUI/Module/EditorUIModule.h"
#include "Modules/Assets/Module/AssetTag.h"

#include <vector>
#include <cstdint>

namespace Kargono::EditorUI
{
	using HexCode = uint32_t;

	struct Color
	{
	public:
		FixedBufStr32 m_Name;
		HexCode m_HexCode;
	};

	struct ColorPaletteMetaData
	{
	public:
		//==============================
		// Serialization
		//==============================
		void Serialize(void* context);
		void Deserialize(void* context);
	public:
		//==============================
		// Public Fields
		//==============================
		FixedBufStr16 m_Name{};
	};

	struct ColorPalette
	{
	public:
		//==============================
		// Metaprogramming Info
		//==============================
		using Metadata = ColorPaletteMetaData;
	public:
		//==============================
		// Static Asset Functions
		//==============================
		constexpr static Assets::AssetConfig GetAssetConfig()
		{
			Assets::AssetConfig config{};
			config.m_Identifier = Assets::GetAssetIdentifier<ColorPalette>();
			config.m_Name = "Color Palette";
			config.m_FileExtension = ".kgpalette";
			config.m_RegistryPath = "ColorPalette/ColorPaletteRegistry.kgreg";
			config.m_Flags.ClearFlag(Assets::AssetFlags::HasAssetCache);
			config.m_Flags.ClearFlag(Assets::AssetFlags::HasIntermediateLocation);
			config.m_Flags.SetFlag(Assets::AssetFlags::HasFileLocation);
			config.m_Flags.ClearFlag(Assets::AssetFlags::HasFileImporting);
			config.m_Flags.SetFlag(Assets::AssetFlags::HasAssetSaving);
			config.m_Flags.SetFlag(Assets::AssetFlags::HasAssetCreationFromName);
			return config;
		}

		static void CreateAssetFileFromName(void* context);
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		ColorPalette() = default;
		~ColorPalette() = default;

	public:
		//==============================
		// Serialization
		//==============================
		void Serialize(void* context);
		void Deserialize(void* context);
	public:
		//==============================
		// Public Fields
		//==============================
		std::vector<Color> m_Colors;
	};

	Register_Module_Type(ColorPalette, Assets::AssetTag)
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
