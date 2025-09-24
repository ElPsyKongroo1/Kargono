#include "kgpch.h"

#include "Modules/Assets/AssetService.h"
#include "Modules/Assets/CustomComponentManager.h"
#include "Kargono/Scenes/Scene.h"

#include "Modules/ECSInternal/CustomComponent.h"

namespace Kargono::Assets
{
	Ref<void> CustomComponentManager::SaveAssetValidation(Ref<ECSInternal::CustomComponent> newAssetRef, AssetHandle assetHandle)
	{
		
	}
	void CustomComponentManager::CreateAssetFileFromName(std::string_view name, AssetInfo& assetInfo, const std::filesystem::path& assetPath)
	{
		// Create new custom component
		Ref<ECSInternal::CustomComponent> newCustomComponent = CreateRef<ECSInternal::CustomComponent>();
		newCustomComponent->m_Name = name;

		// Get identifier
		ECSInternal::ComponentIdentifier identifier = newCustomComponent->RevalidateIdentifier();
		newCustomComponent->m_Identifier = identifier;

		// Save into File
		SerializeAsset(newCustomComponent, assetPath);

		// Load data into In-Memory Metadata object
		Ref<Assets::CustomComponentMetaData> metadata = CreateRef<Assets::CustomComponentMetaData>();
		metadata->Name = name;
		assetInfo.Data.SpecificFileData = metadata;
	}

	void CustomComponentManager::SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::AssetInfo& currentAsset)
	{
		Assets::CustomComponentMetaData* metadata = currentAsset.Data.GetSpecificMetaData<CustomComponentMetaData>();
		serializer << YAML::Key << "Name" << YAML::Value << metadata->Name;
	}
	void CustomComponentManager::DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::AssetInfo& currentAsset)
	{
		Ref<Assets::CustomComponentMetaData> metadata = CreateRef<Assets::CustomComponentMetaData>();
		metadata->Name = metadataNode["Name"].as<std::string>();
		currentAsset.Data.SpecificFileData = metadata;
	}
	void CustomComponentManager::DeleteAssetValidation(AssetHandle assetHandle)
	{
		
	}
}
