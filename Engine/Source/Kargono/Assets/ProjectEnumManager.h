#pragma once
#include "Kargono/Assets/AssetManager.h"

#include <vector>
#include <limits>

namespace Kargono::ProjectData { struct ProjectEnum; }

namespace Kargono::Assets
{
	class ProjectEnumManager : public AssetManager<ProjectData::ProjectEnum>
	{
	public:
		ProjectEnumManager() : AssetManager<ProjectData::ProjectEnum>()
		{
			m_AssetName = "Project Enum";
			m_AssetType = AssetType::ProjectEnum;
			m_FileExtension = ".kgenum";
			m_RegistryLocation = "ProjectEnum/ProjectEnumRegistry.kgreg";
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, false);
			m_Flags.set(AssetManagerOptions::HasFileLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileImporting, false);
			m_Flags.set(AssetManagerOptions::HasAssetSaving, true);
			m_Flags.set(AssetManagerOptions::HasAssetCreationFromName, true);
			m_Flags.set(AssetManagerOptions::HasAssetCache, true);
		}
		virtual ~ProjectEnumManager() = default;
	public:
		// Class specific functions
		virtual void CreateAssetFileFromName(std::string_view name, AssetInfo& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAsset(Ref<ProjectData::ProjectEnum> assetReference, const std::filesystem::path& assetPath) override;
		virtual Ref<ProjectData::ProjectEnum> DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::AssetInfo& currentAsset) override;
		virtual void DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::AssetInfo& currentAsset) override;
	};
}
