#include "kgpch.h"

#include "Modules/EditorUI/Assets/ColorPalette.h"

namespace Kargono::EditorUI
{
	void ColorPalette::Serialize(void* context)
	{
		// Get context
		Assets::SerializeAssetContext* serializeContext = (Assets::SerializeAssetContext*)context;
		KG_ASSERT(serializeContext);
		Assets::Metadata* metadata{ serializeContext->m_AssetMetadata };
		KG_ASSERT(metadata);

		// Get asset path
		const std::filesystem::path& assetPath{ metadata->GetAssetFullFilePath<ColorPalette>() };

		// Serialize asset
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map
		out << YAML::Key << "Colors" << YAML::Value;
		out << YAML::BeginSeq; // Start of Color Sequence
		for (Color color : m_Colors)
		{
			out << YAML::BeginMap; // Start color map
			out << YAML::Key << "Name" << YAML::Value << color.m_Name;
			out << YAML::Key << "HexCode" << YAML::Value << color.m_HexCode;
			out << YAML::EndMap; // End color map
		}
		out << YAML::EndSeq; // End of Color Sequence
		out << YAML::EndMap; // Start of File Map
		// Write to file
		std::ofstream fout(assetPath);
		fout << out.c_str();
		KG_INFO("Successfully Serialized ColorPalette at {}", assetPath);
	}
	void ColorPalette::Deserialize(void* context)
	{
		// Get context
		Assets::DeserializeAssetContext* deserializeContext = (Assets::DeserializeAssetContext*)context;
		KG_ASSERT(deserializeContext);
		Assets::Metadata* metadata{ deserializeContext->m_AssetMetadata };
		KG_ASSERT(metadata);

		// Get asset path
		const std::filesystem::path& assetPath{ metadata->GetAssetFullFilePath<ColorPalette>() };

		// Deserialize asset
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(assetPath.string());
		}
		catch (YAML::ParserException e)
		{
			KG_WARN("Failed to load .kgui file '{0}'\n     {1}", assetPath, e.what());
			return;
		}

		// Get Data Types
		size_t dataSize{ 0 };
		YAML::Node colorNodes = data["Colors"];
		if (colorNodes)
		{
			std::vector<Color>& colorList = m_Colors;
			for (const YAML::Node& colorNode : colorNodes)
			{
				Color& newColor = colorList.emplace_back();
				newColor.m_Name = colorNode["Name"].as<std::string>();
				newColor.m_HexCode = colorNode["HexCode"].as<uint32_t>();
			}
		}
	}

	void ColorPalette::CreateAssetFromName(Assets::Metadata& metadata)
	{
		// Serialize default color palette to file
		ColorPalette defaultColorPalette;
		Assets::SerializeAssetContext serializeContext{ &metadata };
		defaultColorPalette.Serialize((void*)&serializeContext);
	}
}