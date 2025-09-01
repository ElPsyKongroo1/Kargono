#include "kgpch.h"

#include "Modules/Assets/AssetService.h"
#include "Modules/Assets/ProjectComponentManager.h"
#include "Kargono/Scenes/Scene.h"

#include "Modules/ECS/ProjectComponent.h"

namespace Kargono::Assets
{
	Ref<void> ProjectComponentManager::SaveAssetValidation(Ref<ECS::ProjectComponent> newAssetRef, AssetHandle assetHandle)
	{

		// Get old assetInfo reference
		AssetInfo assetInfo = GetAssetRegistry().at(assetHandle);
		std::filesystem::path assetPath =
			(m_Flags.test(AssetManagerOptions::HasIntermediateLocation) ?
				Projects::ProjectService::GetActiveIntermediateDirectory() / assetInfo.Data.IntermediateLocation :
				Projects::ProjectService::GetActiveAssetDirectory() / assetInfo.Data.FileLocation);
		Ref<ECS::ProjectComponent> oldAssetRef = DeserializeAsset(assetInfo, assetPath);

		// Create reallocation instructions which stores information for transferring data from old entity components to new entity components
		Ref<FieldReallocationInstructions> newReallocationInstructions = CreateRef<FieldReallocationInstructions>();

		// Store old types/locations and new types/locations
		newReallocationInstructions->m_OldDataTypes = oldAssetRef->m_DataTypes;
		newReallocationInstructions->m_OldDataLocations = oldAssetRef->m_DataLocations;

		newReallocationInstructions->m_NewDataTypes = newAssetRef->m_DataTypes;
		newReallocationInstructions->m_NewDataLocations = newAssetRef->m_DataLocations;

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
	void ProjectComponentManager::CreateAssetFileFromName(std::string_view name, AssetInfo& assetInfo, const std::filesystem::path& assetPath)
	{
		// Create new project component
		Ref<ECS::ProjectComponent> newProjectComponent = CreateRef<ECS::ProjectComponent>();
		newProjectComponent->m_Name = name;

		// Get identifier
		std::string identifierStr{ "ProjectComponent" "::" + newProjectComponent->m_Name };
		ECSInternal::ComponentIdentifier identifier =
			Utility::FileSystem::CRCFromString(identifierStr.c_str());

		newProjectComponent->m_Identifier = identifier;

		// Save into File
		SerializeAsset(newProjectComponent, assetPath);

		// Load data into In-Memory Metadata object
		Ref<Assets::ProjectComponentMetaData> metadata = CreateRef<Assets::ProjectComponentMetaData>();
		metadata->Name = name;
		assetInfo.Data.SpecificFileData = metadata;
	}
	void ProjectComponentManager::SerializeAsset(Ref<ECS::ProjectComponent> assetReference, const std::filesystem::path& assetPath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map
		// Save name
		out << YAML::Key << "Name" << YAML::Value << assetReference->m_Name;

		// Save size information
		out << YAML::Key << "ComponentSize" << YAML::Value << assetReference->m_ComponentSize;
		out << YAML::Key << "Identifier" << YAML::Value << assetReference->m_Identifier;

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
		for (uint64_t location : assetReference->m_DataLocations)
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
	Ref<ECS::ProjectComponent> ProjectComponentManager::DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath)
	{
		UNREFERENCED_PARAMETER(asset);

		Ref<ECS::ProjectComponent> newProjectComponent = CreateRef<ECS::ProjectComponent>();
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

		// Get name
		newProjectComponent->m_Name = data["Name"].as<std::string>();

		// Get component size information
		newProjectComponent->m_ComponentSize = data["ComponentSize"].as<uint64_t>();
		newProjectComponent->m_Identifier = data["Identifier"].as<ECSInternal::ComponentIdentifier>();

		// Get Data Types
		YAML::Node dataTypesNode = data["DataTypes"];
		if (dataTypesNode)
		{
			std::vector<WrappedVarType>& newTypesList = newProjectComponent->m_DataTypes;
			for (auto dataTypeNode : dataTypesNode)
			{
				newTypesList.push_back(Utility::StringToWrappedVarType(dataTypeNode.as<std::string>()));
			}
		}

		// Get data locations
		YAML::Node dataLocationsNode = data["DataLocations"];
		if (dataLocationsNode)
		{
			std::vector<uint64_t>& newLocationsList = newProjectComponent->m_DataLocations;
			for (auto dataLocationNode : dataLocationsNode)
			{
				newLocationsList.push_back(dataLocationNode.as<uint64_t>());
			}
		}

		// Get data names
		YAML::Node dataNamesNode = data["DataNames"];
		if (dataNamesNode)
		{
			std::vector<FixedBufStr32>& newNamesList{ newProjectComponent->m_DataNames };
			for (const YAML::Node& dataNameNode : dataNamesNode)
			{
				newNamesList.push_back(dataNameNode.as<std::string>().c_str());
			}
		}
		
		return newProjectComponent;
	}

	void ProjectComponentManager::SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::AssetInfo& currentAsset)
	{
		Assets::ProjectComponentMetaData* metadata = currentAsset.Data.GetSpecificMetaData<ProjectComponentMetaData>();
		serializer << YAML::Key << "Name" << YAML::Value << metadata->Name;
	}
	void ProjectComponentManager::DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::AssetInfo& currentAsset)
	{
		Ref<Assets::ProjectComponentMetaData> metadata = CreateRef<Assets::ProjectComponentMetaData>();
		metadata->Name = metadataNode["Name"].as<std::string>();
		currentAsset.Data.SpecificFileData = metadata;
	}
	void ProjectComponentManager::DeleteAssetValidation(AssetHandle assetHandle)
	{
		Ref<ECS::ProjectComponent> deleteComponentRef = GetAsset(assetHandle);
		KG_ASSERT(deleteComponentRef);

		
		// Handle deleting the project component by removing entity data from all scenes
		for (auto& [sceneHandle, assetInfo] : Assets::AssetService::GetSceneRegistry())
		{
			// Get scene
			Ref<Scenes::Scene> currentScene = Assets::AssetService::GetScene(sceneHandle);

			bool sceneModified = Assets::AssetService::RemoveProjectComponentFromScene(currentScene, assetHandle);

			if (sceneModified)
			{
				// Save scene asset on-disk 
				Assets::AssetService::SaveScene(sceneHandle, currentScene);
			}

		}
	}
}
