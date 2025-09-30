#pragma once

#include "Kargono/Math/Math.h"
#include "Modules/Rendering/RenderingService.h"
#include "Modules/RuntimeUI/Module/RuntimeUIModule.h"

#include <string>
#include <string_view>
#include <filesystem>
#include <vector>

namespace Kargono::Rendering { class Texture2D; }

namespace Kargono::RuntimeUI
{
	struct Character
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		Character() = default;
		~Character() = default;
	public:
		//==============================
		// Public Fields
		//==============================
		Math::vec2	m_Size;
		Math::vec2 m_TexCoordinateMin;
		Math::vec2 m_TexCoordinateMax;
		Math::vec2 m_QuadMin;
		Math::vec2 m_QuadMax;
		float m_Advance;
	};

	struct MultiLineTextDimensions
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		MultiLineTextDimensions() = default;
		~MultiLineTextDimensions() = default;
	public:
		//==============================
		// Public Fields
		//==============================
		std::vector<Math::vec2> m_LineSize;
		std::vector<Math::ivec2> m_LineBreaks;
	};

	struct FontMetaData
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		FontMetaData() = default;
		~FontMetaData() = default;
	public:
		//==============================
		// Serialization
		//==============================
		void Deserialize(void* context);
		void Serialize(void* context);
	public:
		//==============================
		// Public Fields
		//==============================
		float m_AtlasWidth{ 0.0f };
		float m_AtlasHeight{ 0.0f };
		float m_LineHeight{ 0.0f };
		float m_Ascender{ 0.0f };
		float m_Descender{ 0.0f };
		std::vector<std::pair<unsigned char, RuntimeUI::Character>> m_Characters{};
	};

	class Font
	{
	public:
		//==============================
		// Metaprogramming Info
		//==============================
		using Metadata = FontMetaData;
	public:
		//==============================
		// Static Asset Functions
		//==============================
		constexpr static Assets::AssetConfig GetAssetConfig()
		{
			Assets::AssetConfig config{};
			config.m_Identifier = Assets::GetAssetIdentifier<Font>();
			config.m_Name = "Font";
			config.m_FileExtension = ".kgfont";
			config.m_ImportExtensions = { ".ttf" };
			config.m_RegistryPath = "Font/FontRegistry.kgreg";
			config.m_Flags.SetFlag(Assets::AssetFlags::HasAssetCache);
			config.m_Flags.SetFlag(Assets::AssetFlags::HasIntermediateLocation);
			config.m_Flags.SetFlag(Assets::AssetFlags::HasFileLocation);
			config.m_Flags.SetFlag(Assets::AssetFlags::HasFileImporting);
			config.m_Flags.ClearFlag(Assets::AssetFlags::HasAssetSaving);
			config.m_Flags.SetFlag(Assets::AssetFlags::HasAssetCreationFromName);
			return config;
		}
		static void CreateAssetFileFromName(std::string_view name,
			Assets::Metadata& metadata, std::filesystem::path& assetPath);
		static void CreateAssetIntermediateFromFile(Assets::Metadata& metadata,
			std::filesystem::path& filePath, std::filesystem::path& intermediatePath);
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		Font() = default;
		~Font() = default;
	public:
		//==============================
		// Serialization
		//==============================
		void Serialize(void* context);
		void Deserialize(void* context);
	public:
		//==============================
		// Rendering
		//==============================
		void OnRenderMultiLineText(std::string_view string, Math::vec3 translation, const glm::vec4& color, float scale = 1.0f, int maxLineWidth = 0);
		void OnRenderSingleLineText(std::string_view string, Math::vec3 translation, const glm::vec4& color, float scale = 1.0f);
	public:
		//==============================
		// Query Values
		//==============================
		Math::vec2 GetSingleLineTextSize(std::string_view text);
		size_t GetIndexFromMousePosition(std::string_view text, float textStartPoint, float mouseXPosition, float textScalingFactor);
		void GetMultiLineTextMetadata(std::string_view text, MultiLineTextDimensions& metadata, float scale, int maxLineWidth = 0);
	public:
		//==============================
		// Public Fields
		//==============================
		Ref<Rendering::Texture2D> m_AtlasTexture{ nullptr };
		float m_LineHeight{ 0.0f };
		std::unordered_map<unsigned char, Character> m_Characters{};
		float m_Ascender{ 0.0f };
		float m_Descender{ 0.0f };
	};

	Register_Module_Type(Font, Assets::AssetTag)
}
