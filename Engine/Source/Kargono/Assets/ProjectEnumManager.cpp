#include "kgpch.h"

#include "Kargono/Assets/AssetService.h"
#include "Kargono/Assets/ProjectEnumManager.h"

#include "Kargono/ProjectData/ProjectEnum.h"

namespace Kargono::Assets
{
	void ProjectEnumManager::CreateAssetFileFromName(std::string_view name, AssetInfo& assetInfo, const std::filesystem::path& assetPath)
	{
		// Create new project component
		Ref<ProjectData::ProjectEnum> newProjectEnum = CreateRef<ProjectData::ProjectEnum>();
		newProjectEnum->m_EnumName = name;

		// Save into File
		SerializeAsset(newProjectEnum, assetPath);

		// Load data into In-Memory Metadata object
		Ref<Assets::ProjectEnumMetaData> metadata = CreateRef<Assets::ProjectEnumMetaData>();
		metadata->Name = name;
		assetInfo.Data.SpecificFileData = metadata;
	}
	void ProjectEnumManager::SerializeAsset(Ref<ProjectData::ProjectEnum> assetReference, const std::filesystem::path& assetPath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of file map
		// Save name
		out << YAML::Key << "Name" << YAML::Value << assetReference->m_EnumName;

		// Save data types
		out << YAML::Key << "Identifiers" << YAML::Value;
		out << YAML::BeginSeq; // Start of enum data sequence
		for (FixedString32 enumerationName : assetReference->m_EnumIdentifiers)
		{
			out << YAML::Value << enumerationName;
		}
		out << YAML::EndSeq; // End of enum data sequence

		out << YAML::EndMap; // End of file map

		std::ofstream fout(assetPath);
		fout << out.c_str();
	}
	Ref<ProjectData::ProjectEnum> ProjectEnumManager::DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath)
	{
		UNREFERENCED_PARAMETER(asset);

		Ref<ProjectData::ProjectEnum> newProjectEnum = CreateRef<ProjectData::ProjectEnum>();
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
		newProjectEnum->m_EnumName = data["Name"].as<std::string>();

		// Get Data Types
		YAML::Node enumDataNode = data["Identifiers"];
		if (enumDataNode)
		{
			std::vector<FixedString32>& newTypesList = newProjectEnum->m_EnumIdentifiers;
			for (const YAML::Node& enumerationNameNode : enumDataNode)
			{
				newTypesList.push_back(enumerationNameNode.as<std::string>().c_str());
			}
		}

		return newProjectEnum;
	}

	void ProjectEnumManager::SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::AssetInfo& currentAsset)
	{
		Assets::ProjectEnumMetaData* metadata = currentAsset.Data.GetSpecificMetaData<ProjectEnumMetaData>();
		serializer << YAML::Key << "Name" << YAML::Value << metadata->Name;
	}
	void ProjectEnumManager::DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::AssetInfo& currentAsset)
	{
		Ref<Assets::ProjectEnumMetaData> metadata = CreateRef<Assets::ProjectEnumMetaData>();
		metadata->Name = metadataNode["Name"].as<std::string>();
		currentAsset.Data.SpecificFileData = metadata;
	}
}
