#pragma once
#include "Modules/Assets/AssetManager.h"

namespace Kargono::ProjectData { struct ColorPalette; }

namespace Kargono::Assets
{
	class ColorPaletteManager : public AssetManager<ProjectData::ColorPalette>
	{
	public:
		
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
