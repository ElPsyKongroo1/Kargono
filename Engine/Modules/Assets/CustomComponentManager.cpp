#include "kgpch.h"

#include "Modules/Assets/AssetService.h"
#include "Modules/Assets/CustomComponentManager.h"
#include "Kargono/Scenes/Scene.h"

#include "Modules/ECSInternal/CustomComponent.h"

namespace Kargono::Assets
{
	Ref<void> CustomComponentManager::SaveAssetValidation(Ref<ECSInternal::CustomComponent> newAssetRef, AssetHandle assetHandle)
	{

		// Get old assetInfo reference
		AssetInfo assetInfo = GetAssetRegistry().at(assetHandle);
		std::filesystem::path assetPath =
			(m_Flags.test(AssetManagerOptions::HasIntermediateLocation) ?
				Projects::ProjectService::GetActiveIntermediateDirectory() / assetInfo.Data.IntermediateLocation :
				Projects::ProjectService::GetActiveAssetDirectory() / assetInfo.Data.FileLocation);
		Ref<ECSInternal::CustomComponent> oldAssetRef = DeserializeAsset(assetInfo, assetPath);

		// Create reallocation instructions which stores information for transferring data from old entity components to new entity components
		Ref<FieldReallocationInstructions> newReallocationInstructions = CreateRef<FieldReallocationInstructions>();

		// Store old types/locations and new types/locations
		newReallocationInstructions->m_OldDataTypes = oldAssetRef->m_DataTypes;
		newReallocationInstructions->m_OldDataLocations = oldAssetRef->m_DataOffsets;

		newReallocationInstructions->m_NewDataTypes = newAssetRef->m_DataTypes;
		newReallocationInstructions->m_NewDataLocations = newAssetRef->m_DataOffsets;

		newReallocationInstructions->m_NewDataSize = newAssetRef->m_ComponentSize;
		for (auto& [sceneHandle, asset] : Assets::AssetService::GetSceneRegistry())
		{
			newReallocationInstructions->m_OldScenes.push_back(Assets::AssetService::GetScene(sceneHandle));
			newReallocationInstructions->m_OldSceneHandles.push_back(sceneHandle);
		}

		// Fill field transfer directions, which maps field data from the old component layout to the new component layout
		for (size_t OuterIteration{ 0 }; OuterIteration < newAssetRef->m_DataNames.size(); OuterIteration++)
		{
			// Check if identical field name exists inside old buffer and ensure similar type. If true, store the location of data in the old buffer
			bool oldBufferContainsField = false;
			for (size_t InnerIteration{ 0 }; InnerIteration < oldAssetRef->m_DataNames.size(); InnerIteration++)
			{
				if (newAssetRef->m_DataNames.at(OuterIteration) == oldAssetRef->m_DataNames.at(InnerIteration) &&
					newAssetRef->m_DataTypes.at(OuterIteration) == oldAssetRef->m_DataTypes.at(InnerIteration))
				{
					oldBufferContainsField = true;
					newReallocationInstructions->m_FieldTransferDirections.push_back(InnerIteration);
					break;
				}
			}

			// If no suitable option is found to get data from, initialize new data (use new allocation index)
			if (!oldBufferContainsField)
			{
				newReallocationInstructions->m_FieldTransferDirections.push_back(k_NewAllocationIndex);
			}

		}

		return newReallocationInstructions;
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
	void CustomComponentManager::SerializeAsset(Ref<ECSInternal::CustomComponent> assetReference, const std::filesystem::path& assetPath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map
		// Save name/identifier
		out << YAML::Key << "Name" << YAML::Value << assetReference->m_Name;
		out << YAML::Key << "Identifier" << YAML::Value << assetReference->m_Identifier;

		// Save size information
		out << YAML::Key << "ComponentSize" << YAML::Value << assetReference->m_ComponentSize;
		out << YAML::Key << "ComponentAlignment" << YAML::Value << assetReference->m_ComponentAlignment;

		// Save data types
		out << YAML::Key << "DataTypes" << YAML::Value;
		out << YAML::BeginSeq; // Start of Data Type Sequence
		for (WrappedVarType type : assetReference->m_DataTypes)
		{
			out << YAML::Value << Utility::WrappedVarTypeToString(type);
		}
		out << YAML::EndSeq; // End of Data Type Sequence

		// Save data locations
		out << YAML::Key << "DataLocations" << YAML::Value;
		out << YAML::BeginSeq; // Start of Data Locations Sequence
		for (uint64_t location : assetReference->m_DataOffsets)
		{
			out << YAML::Value << location;
		}
		out << YAML::EndSeq; // End of Data Locations Sequence

		// Save data names
		out << YAML::Key << "DataNames" << YAML::Value;
		out << YAML::BeginSeq; // Start of Data Names Sequence
		for (FixedBufStr32& name : assetReference->m_DataNames)
		{
			out << YAML::Value << name.CString();
		}
		out << YAML::EndSeq; // End of Data Names Sequence

		out << YAML::EndMap; // Start of File Map

		std::ofstream fout(assetPath);
		fout << out.c_str();
	}
	Ref<ECSInternal::CustomComponent> CustomComponentManager::DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath)
	{
		UNREFERENCED_PARAMETER(asset);

		Ref<ECSInternal::CustomComponent> newCustomComponent = CreateRef<ECSInternal::CustomComponent>();
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(assetPath.string());
		}
		catch (YAML::ParserException e)
		{
			KG_WARN("Failed to load .kgui file '{0}'\n     {1}", assetPath, e.what());
			return nullptr;
		}

		// Get name / identifier
		newCustomComponent->m_Name = data["Name"].as<std::string>();
		newCustomComponent->m_Identifier = data["Identifier"].as<ECSInternal::ComponentIdentifier>();

		// Get component size information
		newCustomComponent->m_ComponentSize = data["ComponentSize"].as<uint64_t>();
		newCustomComponent->m_ComponentAlignment = data["ComponentAlignment"].as<uint64_t>();

		// Get Data Types
		YAML::Node dataTypesNode = data["DataTypes"];
		if (dataTypesNode)
		{
			std::vector<WrappedVarType>& newTypesList = newCustomComponent->m_DataTypes;
			for (auto dataTypeNode : dataTypesNode)
			{
				newTypesList.push_back(Utility::StringToWrappedVarType(dataTypeNode.as<std::string>()));
			}
		}

		// Get data locations
		YAML::Node dataLocationsNode = data["DataLocations"];
		if (dataLocationsNode)
		{
			std::vector<uint64_t>& newLocationsList = newCustomComponent->m_DataOffsets;
			for (auto dataLocationNode : dataLocationsNode)
			{
				newLocationsList.push_back(dataLocationNode.as<uint64_t>());
			}
		}

		// Get data names
		YAML::Node dataNamesNode = data["DataNames"];
		if (dataNamesNode)
		{
			std::vector<FixedBufStr32>& newNamesList{ newCustomComponent->m_DataNames };
			for (const YAML::Node& dataNameNode : dataNamesNode)
			{
				newNamesList.push_back(dataNameNode.as<std::string>().c_str());
			}
		}
		
		return newCustomComponent;
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
		Ref<ECSInternal::CustomComponent> deleteComponentRef = GetAsset(assetHandle);
		KG_ASSERT(deleteComponentRef);

		
		// Handle deleting the custom component by removing entity data from all scenes
		for (auto& [sceneHandle, assetInfo] : Assets::AssetService::GetSceneRegistry())
		{
			// Get scene
			Ref<Scenes::Scene> currentScene = Assets::AssetService::GetScene(sceneHandle);

			bool sceneModified = Assets::AssetService::RemoveCustomComponentFromScene(currentScene, assetHandle);

			if (sceneModified)
			{
				// Save scene asset on-disk 
				Assets::AssetService::SaveScene(sceneHandle, currentScene);
			}

		}
	}
}
