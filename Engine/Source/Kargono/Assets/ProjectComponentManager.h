#pragma once
#include "Kargono/Assets/AssetManager.h"

namespace Kargono::ECS { struct ProjectComponent; }

namespace Kargono::Assets
{
	class ProjectComponentManager : public AssetManager<ECS::ProjectComponent>
	{
	public:
		ProjectComponentManager() : AssetManager<ECS::ProjectComponent>()
		{
			m_AssetName = "Project Component";
			m_AssetType = AssetType::ProjectComponent;
			m_FileExtension = ".kgcomponent";
			m_RegistryLocation = "ProjectComponent/ProjectComponentRegistry.kgreg";
			m_Flags.set(AssetManagerOptions::HasAssetCache, false);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, false);
			m_Flags.set(AssetManagerOptions::HasFileLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileImporting, false);
			m_Flags.set(AssetManagerOptions::HasAssetSaving, true);
			m_Flags.set(AssetManagerOptions::HasAssetCreationFromName, true);
		}
		virtual ~ProjectComponentManager() = default;
	public:
		// Class specific functions
		virtual void CreateAssetFileFromName(const std::string& name, Asset& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAsset(Ref<ECS::ProjectComponent> assetReference, const std::filesystem::path& assetPath) override;
		virtual Ref<ECS::ProjectComponent> DeserializeAsset(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::Asset& currentAsset) override;
		virtual void DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::Asset& currentAsset) override;
	};
}
