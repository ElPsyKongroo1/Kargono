#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class SceneManager : public AssetManagerTemp<Scenes::Scene>
	{
	public:
		SceneManager() : AssetManagerTemp<Scenes::Scene>()
		{
			m_AssetName = "Scene";
			m_AssetType = AssetType::Scene;
			m_FileExtension = ".kgscene";
			m_RegistryLocation = "Scenes/SceneRegistry.kgreg";
			m_Flags.set(AssetManagerOptions::HasAssetCache, false);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, false);
			m_Flags.set(AssetManagerOptions::HasFileLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileImporting, false);
			m_Flags.set(AssetManagerOptions::HasAssetModification, true);
		}
		virtual ~SceneManager() = default;
	public:

		// Class specific functions
		virtual Ref<Scenes::Scene> DeserializeAsset(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::Asset& currentAsset) override;
		virtual void DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::Asset& currentAsset) override;
	};
}
