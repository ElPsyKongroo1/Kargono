#pragma once
#include "Modules/Assets/AssetManager.h"

#include <vector>
#include <limits>

namespace Kargono::ECSInternal { struct CustomComponent; }
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

	class CustomComponentManager : public AssetManager<ECSInternal::CustomComponent>
	{
	public:
		CustomComponentManager() : AssetManager<ECSInternal::CustomComponent>()
		{
			m_AssetName = "Custom Component";
			m_AssetType = AssetType::CustomComponent;
			m_FileExtension = ".kgcomponent";
			m_RegistryLocation = "CustomComponent/CustomComponentRegistry.kgreg";
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, false);
			m_Flags.set(AssetManagerOptions::HasFileLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileImporting, false);
			m_Flags.set(AssetManagerOptions::HasAssetSaving, true);
			m_Flags.set(AssetManagerOptions::HasAssetCreationFromName, true);
			m_Flags.set(AssetManagerOptions::HasAssetCache, true);
		}
		virtual ~CustomComponentManager() = default;
	public:
		// Class specific functions
		virtual Ref<void> SaveAssetValidation(Ref<ECSInternal::CustomComponent> newAsset, AssetHandle assetHandle) override;
		virtual void CreateAssetFileFromName(std::string_view name, AssetInfo& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAsset(Ref<ECSInternal::CustomComponent> assetReference, const std::filesystem::path& assetPath) override;
		virtual Ref<ECSInternal::CustomComponent> DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath) override;
		virtual void SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::AssetInfo& currentAsset) override;
		virtual void DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::AssetInfo& currentAsset) override;
		virtual void DeleteAssetValidation(AssetHandle assetHandle) override;
	};
}
