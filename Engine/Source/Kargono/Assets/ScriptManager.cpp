#include "kgpch.h"

#include "Kargono/Assets/AssetService.h"
#include "Kargono/Assets/ScriptManager.h"
#include "Kargono/Utility/Regex.h"
#include "Kargono/Scripting/ScriptService.h"


namespace Kargono::Assets
{
	std::tuple<AssetHandle, bool> ScriptManager::CreateNewScript(ScriptSpec& spec)
	{
		// Ensure all scripts have unique names
		for (auto& [handle, asset] : m_AssetRegistry)
		{
			Assets::ScriptMetaData metadata = *static_cast<Assets::ScriptMetaData*>(asset.Data.SpecificFileData.get());
			if (metadata.Name == spec.Name)
			{
				KG_WARN("Unable to create new script. Script Name already exists in asset manager");
				return std::make_tuple(0, false);
			}
		}

		// Check if function type is valid
		if (spec.FunctionType == WrappedFuncType::None)
		{
			KG_WARN("Unable to create new script. Invalid Function Type Provided!");
			return std::make_tuple(0, false);
		}

		// If script is associated with a class, ensure class exists
		Assets::AssetHandle classHandle {0};
		if (spec.Type == Scripting::ScriptType::Class)
		{
			if (spec.SectionLabel == "None")
			{
				KG_WARN("Unable to create new script. Empty Section label in spec.");
				return std::make_tuple(0, false);
			}

			bool isValid = false;
			for (auto& [handle, asset] : AssetService::GetEntityClassRegistry())
			{
				if (asset.Data.GetSpecificMetaData<Assets::EntityClassMetaData>()->Name == spec.SectionLabel)
				{
					isValid = true;
					classHandle = handle;
					break;
				}
			}
			if (!isValid)
			{
				KG_WARN("Unable to create new script. No Valid Entity Class Selected!");
				return std::make_tuple(0, false);
			}
		}

		// Create Checksum
		const std::string currentCheckSum {};

		// Create New Asset/Handle
		AssetHandle newHandle{};
		Assets::Asset newAsset{};
		newAsset.Data.Type = AssetType::Script;
		newAsset.Data.FileLocation = m_AssetName + "/" + spec.Name + m_FileExtension;
		newAsset.Data.CheckSum = currentCheckSum;
		newAsset.Handle = newHandle;

		// If script is associated with a class, add script to class's scripts
		if (spec.Type == Scripting::ScriptType::Class)
		{
			Ref<Scenes::EntityClass> entityClass = AssetService::GetEntityClass(classHandle);
			if (!entityClass)
			{
				KG_WARN("Unable to create new script. Could not obtain valid entity class pointer!");
				return std::make_tuple(0, false);
			}

			entityClass->m_Scripts.AllClassScripts.insert(newHandle);
			AssetService::SaveEntityClass(classHandle, entityClass);
		}

		// Create Script File
		FillScriptMetadata(spec, newAsset);
		newAsset.Data.CheckSum = currentCheckSum;

		// Register New Asset and Create Script
		m_AssetRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeAssetRegistry(); // Update Registry File on Disk

		m_AssetCache.insert({ newHandle, DeserializeAsset(newAsset, Projects::ProjectService::GetActiveAssetDirectory() / newAsset.Data.FileLocation) });

		return std::make_tuple(newHandle, true);
	}

	bool ScriptManager::SaveScript(AssetHandle scriptHandle, ScriptSpec& spec)
	{
		// Get original asset/metadata
		Asset asset = m_AssetRegistry.at(scriptHandle);
		ScriptMetaData* metadata = asset.Data.GetSpecificMetaData<ScriptMetaData>();

		// Check if script exists in registry
		if (!m_AssetRegistry.contains(scriptHandle))
		{
			KG_WARN("Unable to update script. Does not exist in registry.");
			return false;
		}

		// Check if an original entity class needs to be updated
		AssetHandle classHandle{ 0 };
		Ref<Scenes::EntityClass> entityClass {nullptr};
		if (spec.Type == Scripting::ScriptType::Class)
		{
			for (auto& [handle, asset] : AssetService::GetEntityClassRegistry())
			{
				if (asset.Data.GetSpecificMetaData<Assets::EntityClassMetaData>()->Name == spec.SectionLabel)
				{
					classHandle = handle;
					break;
				}
			}
			// Cancel if entity class pointer cannot be reached
			entityClass = AssetService::GetEntityClass(classHandle);
			if (!entityClass)
			{
				KG_WARN("Unable to create new script. Could not obtain valid entity class pointer!");
				return false;
			}
		}

		// Check if function type needs to be updated
		std::string scriptFile {};
		std::string matchingExpression {};
		if (metadata->FunctionType != spec.FunctionType)
		{
			// Load file into scriptFile
			scriptFile = Utility::FileSystem::ReadFileString(Projects::ProjectService::GetActiveAssetDirectory() / asset.Data.FileLocation);
			if (scriptFile.empty())
			{
				KG_WARN("Attempt to open script file failed");
				return false;
			}

			// Build Regular Expression
			std::string returnType {Utility::WrappedVarTypeToCPPString(Utility::WrappedFuncTypeToReturnType(metadata->FunctionType))};
			std::string functionName { metadata->Name };
			std::stringstream parameters {};
			for (auto parameter : Utility::WrappedFuncTypeToParameterTypes(metadata->FunctionType))
			{
				std::string parameterString { Utility::WrappedVarTypeToCPPString(parameter) };
				std::string parameterRegex { std::string("\\s*") + parameterString + std::string("\\s+") +
					std::string("\\w+") + std::string("\\s*") + std::string(",?") };
				parameters << parameterRegex;
			}
			matchingExpression = "^" + std::string("\\s*") + returnType +
				std::string("\\s+") + functionName +
				std::string("\\s*") + "\\(" + parameters.str() +
				"\\)";

			// Check if correct number of signatures were found
			uint64_t count = Utility::Regex::GetMatchCount(scriptFile, matchingExpression);
			if (count == 0)
			{
				KG_WARN("Unable to locate function signature when replacing function type in script manager");
				return false;
			}
			if (count > 1)
			{
				KG_WARN("Too many matches for function signature when replacing function type in script manager");
				return false;
			}
		}

		// Update AllScripts inside original Entity Class if needed
		if (metadata->ScriptType == Scripting::ScriptType::Class)
		{
			// Erase Script inside original entity class
			for (auto& [classHandle, asset] : AssetService::GetEntityClassRegistry())
			{
				Ref<Scenes::EntityClass> entityClass = AssetService::GetEntityClass(classHandle);
				if (!entityClass)
				{
					continue;
				}
				if (entityClass->GetScripts().AllClassScripts.contains(scriptHandle))
				{
					entityClass->GetScripts().AllClassScripts.erase(scriptHandle);
					AssetService::SaveEntityClass(classHandle, entityClass);
				}
			}
		}

		// Add script to new class if needed
		if (spec.Type == Scripting::ScriptType::Class)
		{
			entityClass->m_Scripts.AllClassScripts.insert(scriptHandle);
			AssetService::SaveEntityClass(classHandle, entityClass);
		}

		// Update Function Signature if needed
		if (metadata->FunctionType != spec.FunctionType)
		{
			// Replace with new signature
			std::string output = Utility::Regex::ReplaceMatches(scriptFile, matchingExpression,
				Utility::GenerateFunctionSignature(spec.FunctionType, spec.Name));

			// Write back out to file
			Utility::FileSystem::WriteFileString(Projects::ProjectService::GetActiveAssetDirectory() / asset.Data.FileLocation, output);
		}

		// Update registry metadata
		metadata->Name = spec.Name;
		metadata->ScriptType = spec.Type;
		metadata->SectionLabel = spec.SectionLabel;
		metadata->FunctionType = spec.FunctionType;

		SerializeAssetRegistry();

		// Update In-Memory Script
		if (m_AssetCache.contains(scriptHandle))
		{
			Ref<Scripting::Script> script = m_AssetCache.at(scriptHandle);
			script->m_ScriptName = spec.Name;
			script->m_FuncType = spec.FunctionType;
			script->m_ScriptType = spec.Type;
			script->m_SectionLabel = spec.SectionLabel;
			script->m_Function = nullptr;
		}

		return true;
	}

	bool ScriptManager::AddScriptSectionLabel(const std::string& newLabel)
	{
		if (newLabel == "None")
		{
			KG_WARN("Failed to add section label. Cannot add None Label");
			return false;
		}
		if (m_ScriptSectionLabels.contains(newLabel))
		{
			KG_WARN("Failed to add section label. Label already exists in registry");
			return false;
		}

		m_ScriptSectionLabels.insert(newLabel);
		SerializeAssetRegistry();

		return true;
	}

	bool ScriptManager::EditScriptSectionLabel(const std::string& oldLabel, const std::string& newLabel)
	{
		if (!m_ScriptSectionLabels.contains(oldLabel))
		{
			KG_WARN("Failed to delete old section label. Label does not exist in registry");
			return false;
		}

		if (m_ScriptSectionLabels.contains(newLabel))
		{
			KG_WARN("Failed to add new section label. Label already exists in registry");
			return false;
		}

		m_ScriptSectionLabels.erase(oldLabel);
		m_ScriptSectionLabels.insert(newLabel);

		// Change label for all scripts
		for (auto& [handle, script] : m_AssetCache)
		{
			if (script->m_SectionLabel == oldLabel)
			{
				script->m_SectionLabel = newLabel;
			}
		}

		for (auto& [handle, asset] : m_AssetRegistry)
		{
			if (asset.Data.GetSpecificMetaData<ScriptMetaData>()->SectionLabel == oldLabel)
			{
				asset.Data.GetSpecificMetaData<ScriptMetaData>()->SectionLabel = newLabel;
			}
		}

		SerializeAssetRegistry();
		return true;
	}

	bool ScriptManager::DeleteScriptSectionLabel(const std::string& label)
	{
		if (!m_ScriptSectionLabels.contains(label))
		{
			KG_WARN("Failed to delete section label. Label does not exist in registry");
			return false;
		}

		m_ScriptSectionLabels.erase(label);

		// Remove this label from all scripts
		for (auto& [handle, script] : m_AssetCache)
		{
			if (script->m_SectionLabel == label)
			{
				script->m_SectionLabel = "None";
			}
		}

		for (auto& [handle, asset] : m_AssetRegistry)
		{
			if (asset.Data.GetSpecificMetaData<ScriptMetaData>()->SectionLabel == label)
			{
				asset.Data.GetSpecificMetaData<ScriptMetaData>()->SectionLabel = "None";
			}
		}

		SerializeAssetRegistry();

		return true;
	}

	void ScriptManager::FillScriptMetadata(ScriptSpec& spec, Assets::Asset& newAsset)
	{
		// Create script file
		std::filesystem::path fullPath = Projects::ProjectService::GetActiveAssetDirectory() / newAsset.Data.FileLocation;

		Utility::FileSystem::WriteFileString(fullPath, Utility::GenerateFunctionStub(spec.FunctionType, spec.Name));

		// Load data into In-Memory Metadata object
		Ref<Assets::ScriptMetaData> metadata = CreateRef<Assets::ScriptMetaData>();
		metadata->Name = spec.Name;
		metadata->ScriptType = spec.Type;
		metadata->SectionLabel = spec.SectionLabel;
		metadata->FunctionType = spec.FunctionType;
		newAsset.Data.SpecificFileData = metadata;
	}
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
	void ScriptManager::DeleteAssetValidation(AssetHandle scriptHandle)
	{
		// Delete Handle inside associated class
		for (auto& [classHandle, asset] : AssetService::GetEntityClassRegistry())
		{
			Ref<Scenes::EntityClass> entityClass = AssetService::GetEntityClass(classHandle);
			if (!entityClass)
			{
				continue;
			}
			if (entityClass->GetScripts().AllClassScripts.contains(scriptHandle))
			{
				entityClass->GetScripts().AllClassScripts.erase(scriptHandle);
				AssetService::SaveEntityClass(classHandle, entityClass);
			}
		}
	}
}
