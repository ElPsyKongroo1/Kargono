#pragma once
#include "Kargono/Assets/AssetManager.h"

#include <vector>
#include <limits>

namespace Kargono::ECS { struct ProjectComponent; }
namespace Kargono::Scenes { class Scene; }

namespace Kargono::Assets
{
	constexpr size_t k_NewAllocationIndex{ std::numeric_limits<size_t>().max() };

	struct FieldReallocationInstructions
	{
		std::vector<size_t> m_FieldTransferDirections;
		std::vector<WrappedVarType> m_OldDataTypes;
		std::vector<WrappedVarType> m_NewDataTypes;
		std::vector<uint64_t> m_OldDataLocations;
		std::vector<uint64_t> m_NewDataLocations;
		size_t m_NewDataSize;
		std::vector<Ref<Scenes::Scene>> m_OldScenes;
		std::vector<Assets::AssetHandle> m_OldSceneHandles;
	};

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
		virtual Ref<void> SaveAssetValidation(Ref<ECS::ProjectComponent> newAsset, AssetHandle assetHandle) override;
		virtual void CreateAssetFileFromName(const std::string& name, AssetInfo& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAsset(Ref<ECS::ProjectComponent> assetReference, const std::filesystem::path& assetPath) override;
		virtual Ref<ECS::ProjectComponent> DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::AssetInfo& currentAsset) override;
		virtual void DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::AssetInfo& currentAsset) override;
		virtual void DeleteAssetValidation(AssetHandle assetHandle) override;
	};
}
