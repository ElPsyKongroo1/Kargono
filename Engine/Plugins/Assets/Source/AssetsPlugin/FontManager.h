#pragma once
#include "AssetsPlugin/AssetManager.h"

namespace Kargono::RuntimeUI { class Font; }

namespace Kargono::Assets
{
	class FontManager : public AssetManager<RuntimeUI::Font>
	{
	public:
		FontManager() : AssetManager<RuntimeUI::Font>()
		{
			m_AssetName = "Font";
			m_AssetType = AssetType::Font;
			m_FileExtension = ".kgfont";
			m_RegistryLocation = "Font/FontRegistry.kgreg";
			m_ValidImportFileExtensions = { ".ttf" };
			m_Flags.set(AssetManagerOptions::HasAssetCache, true);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileImporting, true);
			m_Flags.set(AssetManagerOptions::HasAssetSaving, false);
			m_Flags.set(AssetManagerOptions::HasAssetCreationFromName, true);
		}
		virtual ~FontManager() = default;
	public:

		// Class specific functions
		virtual Ref<RuntimeUI::Font> DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::AssetInfo& currentAsset) override;
		virtual void CreateAssetFileFromName(std::string_view name, AssetInfo& asset, const std::filesystem::path& assetPath) override;
		virtual void CreateAssetIntermediateFromFile(AssetInfo& newAsset, const std::filesystem::path& fullFileLocation, const std::filesystem::path& fullIntermediateLocation) override;
		virtual void DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::AssetInfo& currentAsset) override;
	};
}
