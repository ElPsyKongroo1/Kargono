#pragma once
#include "Kargono/Assets/AssetManager.h"

namespace Kargono::ProjectData { struct ColorPalette; }

namespace Kargono::Assets
{
	class ColorPaletteManager : public AssetManager<ProjectData::ColorPalette>
	{
	public:
		ColorPaletteManager() : AssetManager<ProjectData::ColorPalette>()
		{
			m_AssetName = "Color Palette";
			m_AssetType = AssetType::ColorPalette;
			m_FileExtension = ".kgpalette";
			m_RegistryLocation = "ColorPalette/ColorPaletteRegistry.kgreg";
			m_Flags.set(AssetManagerOptions::HasAssetCache, false);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, false);
			m_Flags.set(AssetManagerOptions::HasFileLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileImporting, false);
			m_Flags.set(AssetManagerOptions::HasAssetSaving, true);
			m_Flags.set(AssetManagerOptions::HasAssetCreationFromName, true);
		}
		virtual ~ColorPaletteManager() = default;
	public:

		// Class specific functions
		virtual void CreateAssetFileFromName(std::string_view name, AssetInfo& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAsset(Ref<ProjectData::ColorPalette> assetReference, const std::filesystem::path& assetPath) override;
		virtual Ref<ProjectData::ColorPalette> DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::AssetInfo& currentAsset) override;
		virtual void DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::AssetInfo& currentAsset) override;
	};
}
