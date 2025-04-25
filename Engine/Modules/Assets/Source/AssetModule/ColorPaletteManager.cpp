#include "kgpch.h"

#include "AssetModule/AssetService.h"
#include "AssetModule/ColorPaletteManager.h"

#include "Kargono/ProjectData/ColorPalette.h"

namespace Kargono::Assets
{
	void ColorPaletteManager::CreateAssetFileFromName(std::string_view name, AssetInfo& asset, const std::filesystem::path& assetPath)
	{
		// Create Temporary ColorPalette
		Ref<ProjectData::ColorPalette> temporaryColorPalette = CreateRef<ProjectData::ColorPalette>();

		SerializeAsset(temporaryColorPalette, assetPath);

		// Load data into In-Memory Metadata object
		Ref<Assets::ColorPaletteMetaData> metadata = CreateRef<Assets::ColorPaletteMetaData>();
		metadata->Name = name;
		asset.Data.SpecificFileData = metadata;
	}
	void ColorPaletteManager::SerializeAsset(Ref<ProjectData::ColorPalette> assetReference, const std::filesystem::path& assetPath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map

		out << YAML::Key << "Colors" << YAML::Value;
		out << YAML::BeginSeq; // Start of Color Sequence
		for (ProjectData::Color color : assetReference->m_Colors)
		{
			out << YAML::BeginMap; // Start color map
			out << YAML::Key << "Name" << YAML::Value << color.m_Name;
			out << YAML::Key << "HexCode" << YAML::Value << color.m_HexCode;
			out << YAML::EndMap; // End color map
		}
		out << YAML::EndSeq; // End of Color Sequence

		out << YAML::EndMap; // Start of File Map

		std::ofstream fout(assetPath);
		fout << out.c_str();
		KG_INFO("Successfully Serialized ColorPalette at {}", assetPath);
	}
	Ref<ProjectData::ColorPalette> ColorPaletteManager::DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath)
	{
		UNREFERENCED_PARAMETER(asset);

		Ref<ProjectData::ColorPalette> newColorPalette = CreateRef<ProjectData::ColorPalette>();
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(assetPath.string());
		}
		catch (YAML::ParserException e)
		{
			KG_WARN("Failed to load .kgui file '{0}'\n     {1}", assetPath, e.what());
			return nullptr;
		}

		size_t dataSize{ 0 };
		// Get Data Types
		YAML::Node colorNodes = data["Colors"];
		if (colorNodes)
		{
			std::vector<ProjectData::Color>& colorList = newColorPalette->m_Colors;
			for (const YAML::Node& colorNode : colorNodes)
			{
				ProjectData::Color& newColor = colorList.emplace_back();
				newColor.m_Name = colorNode["Name"].as<std::string>();
				newColor.m_HexCode = colorNode["HexCode"].as<uint32_t>();
			}
		}

		return newColorPalette;
	}

	void ColorPaletteManager::SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::AssetInfo& currentAsset)
	{
		Assets::ColorPaletteMetaData* metadata = currentAsset.Data.GetSpecificMetaData<ColorPaletteMetaData>();
		serializer << YAML::Key << "Name" << YAML::Value << metadata->Name;
	}
	void ColorPaletteManager::DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::AssetInfo& currentAsset)
	{
		Ref<Assets::ColorPaletteMetaData> ColorPaletteMetaData = CreateRef<Assets::ColorPaletteMetaData>();
		ColorPaletteMetaData->Name = metadataNode["Name"].as<std::string>();
		currentAsset.Data.SpecificFileData = ColorPaletteMetaData;
	}
}
