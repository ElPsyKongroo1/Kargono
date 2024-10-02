#pragma once
#include "Kargono/Assets/AssetManager.h"

namespace Kargono::Rendering { class Rendering::Texture2D; }

namespace Kargono::Assets
{
	class Texture2DManager : public AssetManager<Rendering::Texture2D>
	{
	public:
		Texture2DManager() : AssetManager<Rendering::Texture2D>()
		{
			m_AssetName = "Texture";
			m_AssetType = AssetType::Texture;
			m_FileExtension = ".kgtexture";
			m_RegistryLocation = "Texture2D/TextureRegistry.kgreg";
			m_ValidImportFileExtensions = { ".png" };
			m_Flags.set(AssetManagerOptions::HasAssetCache, true);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileImporting, true);
			m_Flags.set(AssetManagerOptions::HasAssetSaving, false);
			m_Flags.set(AssetManagerOptions::HasAssetCreationFromName, false);
		}
		virtual ~Texture2DManager() = default;
	public:
		// Class specific functions
		virtual Ref<Rendering::Texture2D> DeserializeAsset(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::Asset& currentAsset) override;
		virtual void CreateAssetIntermediateFromFile(Asset& newAsset, const std::filesystem::path& fullFileLocation, const std::filesystem::path& fullIntermediateLocation) override;
		virtual void DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::Asset& currentAsset) override;

		AssetHandle ImportNewTextureFromData(Buffer buffer, int32_t width, int32_t height, int32_t channels);
		void CreateTextureIntermediateFromBuffer(Buffer buffer, int32_t width, int32_t height, int32_t channels, Assets::Asset& newAsset);
	};
}
