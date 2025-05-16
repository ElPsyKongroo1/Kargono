#include "kgpch.h"

#include "Modules/Assets/AssetService.h"
#include "Modules/Assets/GlobalStateManager.h"

#include "Kargono/ProjectData/GlobalState.h"

namespace Kargono::Assets
{
	void GlobalStateManager::CreateAssetFileFromName(std::string_view name, AssetInfo& asset, const std::filesystem::path& assetPath)
	{
		// Create Temporary GlobalState
		Ref<ProjectData::GlobalState> temporaryGlobalState = CreateRef<ProjectData::GlobalState>();
		temporaryGlobalState->m_Name = name;

		SerializeAsset(temporaryGlobalState, assetPath);

		// Load data into In-Memory Metadata object
		Ref<Assets::GlobalStateMetaData> metadata = CreateRef<Assets::GlobalStateMetaData>();
		metadata->Name = name;
		asset.Data.SpecificFileData = metadata;
	}
	void GlobalStateManager::SerializeAsset(Ref<ProjectData::GlobalState> assetReference, const std::filesystem::path& assetPath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map
		// Save name
		out << YAML::Key << "Name" << YAML::Value << assetReference->m_Name;

		// Save size information
		out << YAML::Key << "BufferSize" << YAML::Value << assetReference->m_DataBuffer.Size;

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
		for (size_t location : assetReference->m_DataLocations)
		{
			out << YAML::Value << location;
		}
		out << YAML::EndSeq; // End of Data Locations Sequence

		// Save data names
		out << YAML::Key << "DataNames" << YAML::Value;
		out << YAML::BeginSeq; // Start of Data Names Sequence
		for (FixedString32& name : assetReference->m_DataNames)
		{
			out << YAML::Value << name.CString();
		}
		out << YAML::EndSeq; // End of Data Names Sequence

		out << YAML::Key << "Data" << YAML::BeginMap; // Start data map
		size_t iteration{ 0 };
		for (WrappedVarType type : assetReference->m_DataTypes)
		{
			Utility::SerializeWrappedVarType
			(
				out, 
				type, 
				assetReference->m_DataNames.at(iteration),
				assetReference->m_DataBuffer.Data + assetReference->m_DataLocations.at(iteration)
			);
			iteration++;
		}
		out << YAML::EndMap; // End data map

		out << YAML::EndMap; // Start of File Map

		std::ofstream fout(assetPath);
		fout << out.c_str();
		KG_INFO("Successfully Serialized GlobalState at {}", assetPath);
	}
	Ref<ProjectData::GlobalState> GlobalStateManager::DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath)
	{
		UNREFERENCED_PARAMETER(asset);

		Ref<ProjectData::GlobalState> newGlobalState = CreateRef<ProjectData::GlobalState>();
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
		newGlobalState->m_Name = data["Name"].as<std::string>();

		// Get component size information
		newGlobalState->m_DataBuffer.Size = data["BufferSize"].as<uint64_t>();

		size_t dataSize{ 0 };
		// Get Data Types
		YAML::Node dataTypesNode = data["DataTypes"];
		if (dataTypesNode)
		{
			std::vector<WrappedVarType>& newTypesList = newGlobalState->m_DataTypes;
			for (const YAML::Node& dataTypeNode : dataTypesNode)
			{
				WrappedVarType type = newTypesList.emplace_back(Utility::StringToWrappedVarType(dataTypeNode.as<std::string>()));
				dataSize += Utility::WrappedVarTypeToDataSizeBytes(type);
			}
		}

		// Get data locations
		YAML::Node dataLocationsNode = data["DataLocations"];
		if (dataLocationsNode)
		{
			std::vector<size_t>& newLocationsList = newGlobalState->m_DataLocations;
			for (const YAML::Node& dataLocationNode : dataLocationsNode)
			{
				newLocationsList.push_back(dataLocationNode.as<size_t>());
			}
		}

		// Get data names
		YAML::Node dataNamesNode = data["DataNames"];
		if (dataNamesNode)
		{
			std::vector<FixedString32>& newNamesList = newGlobalState->m_DataNames;
			for (const YAML::Node& dataNameNode : dataNamesNode)
			{
				newNamesList.push_back(dataNameNode.as<std::string>().c_str());
			}
		}

		// Allocate buffer for new data
		if (dataSize > 0)
		{
			newGlobalState->m_DataBuffer.Allocate(dataSize);
		}

		// Get data
		YAML::Node dataNode = data["Data"];
		if (dataNamesNode)
		{
			size_t iteration{ 0 };
			for (FixedString32& name : newGlobalState->m_DataNames)
			{
				Utility::DeserializeWrappedVarType
				(
					dataNode,
					newGlobalState->m_DataTypes.at(iteration),
					name.CString(),
					newGlobalState->m_DataBuffer.Data + newGlobalState->m_DataLocations.at(iteration)
				);
				iteration++;
			}
		}

		return newGlobalState;
	}
	void GlobalStateManager::SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::AssetInfo& currentAsset)
	{
		Assets::GlobalStateMetaData* metadata = currentAsset.Data.GetSpecificMetaData<GlobalStateMetaData>();
		serializer << YAML::Key << "Name" << YAML::Value << metadata->Name;
	}
	void GlobalStateManager::DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::AssetInfo& currentAsset)
	{
		Ref<Assets::GlobalStateMetaData> GlobalStateMetaData = CreateRef<Assets::GlobalStateMetaData>();
		GlobalStateMetaData->Name = metadataNode["Name"].as<std::string>();
		currentAsset.Data.SpecificFileData = GlobalStateMetaData;
	}
}
