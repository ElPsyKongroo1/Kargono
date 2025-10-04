#include "kgpch.h"

#include "Modules/EditorUI/Assets/ColorPalette.h"

namespace Kargono::EditorUI
{
	void ColorPaletteMetaData::Serialize(void* context)
	{
		// Get context
		Assets::SerializeMetaDataContext* serializeContext = 
			(Assets::SerializeMetaDataContext*)context;
		// Get asset path
		YAML::Emitter& serializer = *serializeContext->m_Serializer;
		// Serialize
		serializer << YAML::Key << "Name" << YAML::Value << m_Name;
	}

	void ColorPaletteMetaData::Deserialize(void* context)
	{
		// Get context
		Assets::DeserializeMetaDataContext* deserializeContext = (Assets::DeserializeMetaDataContext*)context;
		// Get asset path
		KG_ASSERT(deserializeContext->m_Node);
		YAML::Node& metadataNode{ *deserializeContext->m_Node };
		// Deserialize
		m_Name = metadataNode["Name"].as<std::string>();
	}

	void ColorPalette::Serialize(void* context)
	{
		// Get context
		Assets::SerializeAssetContext* serializeContext = (Assets::SerializeAssetContext*)context;
		// Get asset path
		const std::filesystem::path& assetPath = serializeContext->m_AssetPath;
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

		// Get asset path
		const std::filesystem::path& assetPath = deserializeContext->m_AssetPath;

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

	void ColorPalette::CreateAssetFromName(Assets::Metadata& metadata, std::string_view name,
		std::filesystem::path& assetPath)
	{
		// Serialize temporary color palette to file
		ColorPalette temporaryColorPalette;
		Assets::SerializeAssetContext serializeContext{ assetPath };
		temporaryColorPalette.Serialize((void*)&serializeContext);

		// Set metadata fields
		ColorPaletteMetaData* colorPaletteMetadata{ metadata.GetSpecificMetaData<ColorPaletteMetaData>() };
		KG_ASSERT(colorPaletteMetadata);
		colorPaletteMetadata->m_Name = name;
	}
}