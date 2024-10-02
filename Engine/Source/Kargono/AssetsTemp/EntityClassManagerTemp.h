#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class EntityClassManager : public AssetManagerTemp<Scenes::EntityClass>
	{
	public:
		EntityClassManager() : AssetManagerTemp<Scenes::EntityClass>()
		{
			m_AssetName = "Entity Class";
			m_AssetType = AssetType::EntityClass;
			m_FileExtension = ".kgclass";
			m_RegistryLocation = "EntityClass/EntityClassRegistry.kgreg";
			m_Flags.set(AssetManagerOptions::HasAssetCache, false);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, false);
			m_Flags.set(AssetManagerOptions::HasFileLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileImporting, false);
			m_Flags.set(AssetManagerOptions::HasAssetSaving, false);
			m_Flags.set(AssetManagerOptions::HasAssetCreationFromName, true);
		}
		virtual ~EntityClassManager() = default;
	public:

		// Class specific functions
		virtual void CreateAssetFileFromName(const std::string& name, Asset& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAsset(Ref<Scenes::EntityClass> assetReference, const std::filesystem::path& assetPath) override;
		virtual Ref<Scenes::EntityClass> DeserializeAsset(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::Asset& currentAsset) override;
		virtual void DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::Asset& currentAsset) override;

		void SaveEntityClass(AssetHandle entityClassHandle, Ref<Scenes::EntityClass> entityClass, Ref<Scenes::Scene> editorScene);
		void DeleteEntityClass(AssetHandle handle, Ref<Scenes::Scene> editorScene);
	};
}
