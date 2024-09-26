#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class Texture2DManager : public AssetManagerTemp<Rendering::Texture2D>
	{
	public:
		Texture2DManager() : AssetManagerTemp<Rendering::Texture2D>()
		{
			m_AssetName = "Texture";
			m_AssetType = AssetType::Texture;
			m_FileExtension = ".kgtexture";
			m_RegistryLocation = "Textures/Intermediates/TextureRegistry.kgreg";
			m_ValidImportFileExtensions = { ".png" };
			m_Flags.set(AssetManagerOptions::HasAssetCache, true);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileImporting, true);
			m_Flags.set(AssetManagerOptions::HasAssetModification, false);
		}
		virtual ~Texture2DManager() = default;
	public:
		// Class specific functions
		virtual Ref<Rendering::Texture2D> DeserializeAsset(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::Asset& currentAsset) override;
		virtual void CreateAssetIntermediateFromFile(Asset& newAsset, const std::filesystem::path& fullFileLocation, const std::filesystem::path& fullIntermediateLocation) override;
		virtual void DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::Asset& currentAsset) override;
	};
}
