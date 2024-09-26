#include "kgpch.h"

#include "Kargono/AssetsTemp/AssetService.h"
#include "Kargono/AssetsTemp/ScriptManagerTemp.h"
#include "Kargono/Utility/Regex.h"
#include "Kargono/Scripting/ScriptService.h"


namespace Kargono::Assets
{
	Ref<Scripting::Script> ScriptManager::DeserializeAsset(Assets::Asset& asset, const std::filesystem::path& assetPath)
	{
		Ref<Scripting::Script> newScript = CreateRef<Scripting::Script>();
		Assets::ScriptMetaData metadata = *asset.Data.GetSpecificMetaData<ScriptMetaData>();

		newScript->m_ID = asset.Handle;
		newScript->m_ScriptName = metadata.Name;
		newScript->m_FuncType = metadata.FunctionType;
		newScript->m_ScriptType = metadata.ScriptType;
		newScript->m_SectionLabel = metadata.SectionLabel;
		Scripting::ScriptService::LoadScriptFunction(newScript, metadata.FunctionType);

		return newScript;
	}
	void ScriptManager::SerializeRegistrySpecificData(YAML::Emitter& serializer)
	{
		// Section Labels
		serializer << YAML::Key << "SectionLabels" << YAML::Value;
		serializer << YAML::BeginSeq; // Start SectionLabels

		for (auto& section : m_ScriptSectionLabels)
		{
			serializer << YAML::Value << section; // Section Name
		}
	}
	void ScriptManager::SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::Asset& currentAsset)
	{
		Assets::ScriptMetaData* metadata = static_cast<Assets::ScriptMetaData*>(currentAsset.Data.SpecificFileData.get());

		serializer << YAML::Key << "Name" << YAML::Value << metadata->Name;
		serializer << YAML::Key << "SectionLabel" << YAML::Value << metadata->SectionLabel;
		serializer << YAML::Key << "ScriptType" << YAML::Value << Utility::ScriptTypeToString(metadata->ScriptType);
		serializer << YAML::Key << "FunctionType" << YAML::Value << Utility::WrappedFuncTypeToString(metadata->FunctionType);
	}
	void ScriptManager::DeserializeRegistrySpecificData(YAML::Node& registryNode)
	{
		// Load in Engine Scripts
		for (auto script : Scripting::ScriptService::GetAllEngineScripts())
		{
			Assets::Asset newAsset{};
			newAsset.Handle = script->m_ID;

			newAsset.Data.CheckSum = "";
			newAsset.Data.FileLocation = "";
			newAsset.Data.Type = AssetType::Script;

			// Insert ScriptMetaData
			Ref<Assets::ScriptMetaData> ScriptMetaData = CreateRef<Assets::ScriptMetaData>();

			std::string Name{};
			std::vector<WrappedVarType> Parameters{};

			ScriptMetaData->Name = script->m_ScriptName;
			ScriptMetaData->SectionLabel = script->m_SectionLabel;
			ScriptMetaData->ScriptType = script->m_ScriptType;
			ScriptMetaData->FunctionType = script->m_FuncType;
			newAsset.Data.SpecificFileData = ScriptMetaData;

			// Insert Engine Script into registry/in-memory
			m_AssetRegistry.insert({ newAsset.Handle, newAsset });
			m_AssetCache.insert({ newAsset.Handle, script });
		}

		// Get Section Labels
		{
			m_ScriptSectionLabels.clear();
			auto sectionLabels = registryNode["SectionLabels"];
			if (sectionLabels)
			{
				for (auto label : sectionLabels)
				{
					m_ScriptSectionLabels.insert(label.as<std::string>());
				}
			}
		}
	}
	void ScriptManager::DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::Asset& currentAsset)
	{
		Ref<Assets::ScriptMetaData> ScriptMetaData = CreateRef<Assets::ScriptMetaData>();

		std::string Name{};
		std::vector<WrappedVarType> Parameters{};

		ScriptMetaData->Name = metadataNode["Name"].as<std::string>();

		ScriptMetaData->SectionLabel = metadataNode["SectionLabel"].as<std::string>();
		ScriptMetaData->ScriptType = Utility::StringToScriptType(metadataNode["ScriptType"].as<std::string>());
		ScriptMetaData->FunctionType = Utility::StringToWrappedFuncType(metadataNode["FunctionType"].as<std::string>());
		currentAsset.Data.SpecificFileData = ScriptMetaData;
	}
}
