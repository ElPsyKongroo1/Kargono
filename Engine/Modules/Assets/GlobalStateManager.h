#pragma once
#include "Modules/Assets/AssetManager.h"

namespace Kargono::ProjectData { struct GlobalState; }

namespace Kargono::Assets
{
	class GlobalStateManager : public AssetManager<ProjectData::GlobalState>
	{
	public:
		GlobalStateManager() : AssetManager<ProjectData::GlobalState>()
		{
			m_AssetName = "Global State";
			m_AssetType = AssetType::GlobalState;
			m_FileExtension = ".kggstate";
			m_RegistryLocation = "GlobalState/GlobalStateRegistry.kgreg";
			m_Flags.set(AssetManagerOptions::HasAssetCache, false);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, false);
			m_Flags.set(AssetManagerOptions::HasFileLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileImporting, false);
			m_Flags.set(AssetManagerOptions::HasAssetSaving, true);
			m_Flags.set(AssetManagerOptions::HasAssetCreationFromName, true);
		}
		virtual ~GlobalStateManager() = default;
	public:

		// Class specific functions
		virtual void CreateAssetFileFromName(std::string_view name, AssetInfo& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAsset(Ref<ProjectData::GlobalState> assetReference, const std::filesystem::path& assetPath) override;
		virtual Ref<ProjectData::GlobalState> DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::AssetInfo& currentAsset) override;
		virtual void DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::AssetInfo& currentAsset) override;
	};
}
