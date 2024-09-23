#include "kgpch.h"

#include "Kargono/Assets/AssetManager.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Utility/FileSystem.h"
#include "Kargono/Utility/Regex.h"

#include "API/Serialization/yamlcppAPI.h"

namespace Kargono::Assets
{
	std::unordered_map<AssetHandle, Assets::Asset> AssetManager::s_ScriptRegistry {};
	std::unordered_map<AssetHandle, Ref<Scripting::Script>> AssetManager::s_Scripts {};
	std::unordered_set<std::string> AssetManager::s_ScriptSectionLabels {};

	void AssetManager::DeserializeScriptRegistry()
	{
		// Clear current registry and open registry in current project 
		s_ScriptRegistry.clear();
		s_Scripts.clear();
		KG_ASSERT(Projects::ProjectService::GetActive(), "There is no currently loaded project to serialize from!");
		const auto& ScriptRegistryLocation = Projects::ProjectService::GetActiveAssetDirectory() / "Scripting/ScriptRegistry.kgreg";

		// Load in Engine Scripts
		for (auto script : Scripting::ScriptService::GetAllEngineScripts())
		{
			Assets::Asset newAsset{};
			newAsset.Handle = script->m_ID;

			newAsset.Data.CheckSum = "";
			newAsset.Data.IntermediateLocation = "";
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
			s_ScriptRegistry.insert({ newAsset.Handle, newAsset });
			s_Scripts.insert({ newAsset.Handle, script });
		}

		if (!std::filesystem::exists(ScriptRegistryLocation))
		{
			KG_WARN("No .kgregistry file exists in project path!");
			return;
		}
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(ScriptRegistryLocation.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load .kgscene file '{0}'\n     {1}", ScriptRegistryLocation.string(), e.what());
			return;
		}

		// Opening registry node 
		if (!data["Registry"]) { return; }

		std::string registryName = data["Registry"].as<std::string>();
		KG_INFO("Deserializing Script Registry");

		// Get Section Labels
		{
			s_ScriptSectionLabels.clear();
			auto sectionLabels = data["SectionLabels"];
			if (sectionLabels)
			{
				for (auto label : sectionLabels)
				{
					s_ScriptSectionLabels.insert(label.as<std::string>());
				}
			}
		}

		// Opening all assets 
		auto assets = data["Assets"];
		if (assets)
		{
			for (auto asset : assets)
			{
				Assets::Asset newAsset{};
				newAsset.Handle = asset["AssetHandle"].as<uint64_t>();

				// Retrieving metadata for asset 
				auto metadata = asset["MetaData"];
				newAsset.Data.CheckSum = metadata["CheckSum"].as<std::string>();
				newAsset.Data.IntermediateLocation = metadata["IntermediateLocation"].as<std::string>();
				newAsset.Data.Type = Utility::StringToAssetType(metadata["AssetType"].as<std::string>());

				// Retrieving Script specific metadata 
				if (newAsset.Data.Type == Assets::AssetType::Script)
				{
					Ref<Assets::ScriptMetaData> ScriptMetaData = CreateRef<Assets::ScriptMetaData>();

					std::string Name{};
					std::vector<WrappedVarType> Parameters{};

					ScriptMetaData->Name = metadata["Name"].as<std::string>();

					ScriptMetaData->SectionLabel = metadata["SectionLabel"].as<std::string>();
					ScriptMetaData->ScriptType = Utility::StringToScriptType(metadata["ScriptType"].as<std::string>());
					ScriptMetaData->FunctionType = Utility::StringToWrappedFuncType(metadata["FunctionType"].as<std::string>());
					newAsset.Data.SpecificFileData = ScriptMetaData;

				}

				// Add asset to in memory registry 
				s_ScriptRegistry.insert({ newAsset.Handle, newAsset });

				s_Scripts.insert({ newAsset.Handle, InstantiateScriptIntoMemory(newAsset) });
			}
		}
	}

	void AssetManager::SerializeScriptRegistry()
	{
		KG_ASSERT(Projects::ProjectService::GetActive(), "There is no currently loaded project to serialize to!");
		const auto& ScriptRegistryLocation = Projects::ProjectService::GetActiveAssetDirectory() / "Scripting/ScriptRegistry.kgreg";
		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "Registry" << YAML::Value << "Script";

		// Section Labels
		out << YAML::Key << "SectionLabels" << YAML::Value;
		out << YAML::BeginSeq; // Start SectionLabels

		for (auto& section : s_ScriptSectionLabels)
		{
			out << YAML::Value << section; // Section Name
		}

		out << YAML::EndSeq; // End SectionLabels

		// Assets
		out << YAML::Key << "Assets" << YAML::Value << YAML::BeginSeq;
		for (auto& [handle, asset] : s_ScriptRegistry)
		{
			if (asset.Data.GetSpecificFileData<ScriptMetaData>()->ScriptType == Scripting::ScriptType::Engine)
			{
				continue;
			}
			out << YAML::BeginMap; // Asset Map
			out << YAML::Key << "AssetHandle" << YAML::Value << static_cast<uint64_t>(handle);

			out << YAML::Key << "MetaData" << YAML::Value;
			out << YAML::BeginMap; // MetaData Map
			out << YAML::Key << "CheckSum" << YAML::Value << asset.Data.CheckSum;
			out << YAML::Key << "IntermediateLocation" << YAML::Value << asset.Data.IntermediateLocation.string();
			out << YAML::Key << "AssetType" << YAML::Value << Utility::AssetTypeToString(asset.Data.Type);

			if (asset.Data.Type == Assets::AssetType::Script)
			{
				Assets::ScriptMetaData* metadata = static_cast<Assets::ScriptMetaData*>(asset.Data.SpecificFileData.get());

				out << YAML::Key << "Name" << YAML::Value << metadata->Name;
				out << YAML::Key << "SectionLabel" << YAML::Value << metadata->SectionLabel;
				out << YAML::Key << "ScriptType" << YAML::Value << Utility::ScriptTypeToString(metadata->ScriptType);
				out << YAML::Key << "FunctionType" << YAML::Value << Utility::WrappedFuncTypeToString(metadata->FunctionType);
			}

			out << YAML::EndMap; // MetaData Map
			out << YAML::EndMap; // Asset Map
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		Utility::FileSystem::CreateNewDirectory(ScriptRegistryLocation.parent_path());

		std::ofstream fout(ScriptRegistryLocation);
		fout << out.c_str();
	}

	std::tuple<AssetHandle, bool> AssetManager::CreateNewScript(ScriptSpec& spec)
	{
		// Ensure all scripts have unique names
		for (auto& [handle, asset] : s_ScriptRegistry)
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
			for (auto& [handle, asset] : s_EntityClassRegistry)
			{
				if (asset.Data.GetSpecificFileData<Assets::EntityClassMetaData>()->Name == spec.SectionLabel)
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
		newAsset.Handle = newHandle;

		// If script is associated with a class, add script to class's scripts
		if (spec.Type == Scripting::ScriptType::Class)
		{
			Ref<Scenes::EntityClass> entityClass = GetEntityClass(classHandle);
			if (!entityClass)
			{
				KG_WARN("Unable to create new script. Could not obtain valid entity class pointer!");
				return std::make_tuple(0, false);
			}

			entityClass->m_Scripts.AllClassScripts.insert(newHandle);
			SaveEntityClass(classHandle, entityClass);
		}

		// Create Intermediate
		FillScriptMetadata(spec, newAsset);
		newAsset.Data.CheckSum = currentCheckSum;

		// Register New Asset and Create Script
		s_ScriptRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeScriptRegistry(); // Update Registry File on Disk

		s_Scripts.insert({ newHandle, InstantiateScriptIntoMemory(newAsset) });

		return std::make_tuple(newHandle, true);
	}

	bool AssetManager::UpdateScript(AssetHandle scriptHandle, ScriptSpec& spec)
	{
		// Get original asset/metadata
		Asset asset = s_ScriptRegistry.at(scriptHandle);
		ScriptMetaData* metadata = asset.Data.GetSpecificFileData<ScriptMetaData>();

		// Check if script exists in registry
		if (!s_ScriptRegistry.contains(scriptHandle))
		{
			KG_WARN("Unable to update script. Does not exist in registry.");
			return false;
		}

		// Check if an original entity class needs to be updated
		AssetHandle classHandle{ 0 };
		Ref<Scenes::EntityClass> entityClass {nullptr};
		if (spec.Type == Scripting::ScriptType::Class)
		{
			for (auto& [handle, asset] : s_EntityClassRegistry)
			{
				if (asset.Data.GetSpecificFileData<Assets::EntityClassMetaData>()->Name == spec.SectionLabel)
				{
					classHandle = handle;
					break;
				}
			}
			// Cancel if entity class pointer cannot be reached
			entityClass = GetEntityClass(classHandle);
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
			scriptFile = Utility::FileSystem::ReadFileString(Projects::ProjectService::GetActiveAssetDirectory() / asset.Data.IntermediateLocation);
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
			matchingExpression =  "^" + std::string("\\s*") + returnType +
				std::string("\\s+") + functionName +
				std::string("\\s*") + "\\(" + parameters.str() +
				"\\)";

			// Check if correct number of signatures were found
			uint64_t count = Utility::Regex::GetMatchCount(scriptFile, matchingExpression);
			if (count == 0)
			{
				KG_ERROR("Unable to locate function signature when replacing function type in script manager");
				return false;
			}
			if (count > 1)
			{
				KG_ERROR("Too many matches for function signature when replacing function type in script manager");
				return false;
			}
		}

		// Update AllScripts inside original Entity Class if needed
		if (metadata->ScriptType == Scripting::ScriptType::Class)
		{
			// Erase Script inside original entity class
			for (auto& [classHandle, asset] : s_EntityClassRegistry)
			{
				Ref<Scenes::EntityClass> entityClass = GetEntityClass(classHandle);
				if (!entityClass)
				{
					continue;
				}
				if (entityClass->GetScripts().AllClassScripts.contains(scriptHandle))
				{
					entityClass->GetScripts().AllClassScripts.erase(scriptHandle);
					SaveEntityClass(classHandle, entityClass);
				}
			}
		}

		// Add script to new class if needed
		if (spec.Type == Scripting::ScriptType::Class)
		{
			entityClass->m_Scripts.AllClassScripts.insert(scriptHandle);
			SaveEntityClass(classHandle, entityClass);
		}

		// Update Function Signature if needed
		if (metadata->FunctionType != spec.FunctionType)
		{
			// Replace with new signature
			std::string output = Utility::Regex::ReplaceMatches(scriptFile, matchingExpression, 
				Utility::GenerateFunctionSignature(spec.FunctionType, spec.Name));
			
			// Write back out to file
			Utility::FileSystem::WriteFileString(Projects::ProjectService::GetActiveAssetDirectory() / asset.Data.IntermediateLocation, output);
		}

		// Update registry metadata
		metadata->Name = spec.Name;
		metadata->ScriptType = spec.Type;
		metadata->SectionLabel = spec.SectionLabel;
		metadata->FunctionType = spec.FunctionType;

		SerializeScriptRegistry();

		// Update In-Memory Script
		if (s_Scripts.contains(scriptHandle))
		{
			Ref<Scripting::Script> script = s_Scripts.at(scriptHandle);
			script->m_ScriptName = spec.Name;
			script->m_FuncType = spec.FunctionType;
			script->m_ScriptType = spec.Type;
			script->m_SectionLabel = spec.SectionLabel;
			script->m_Function = nullptr;
		}

		return true;
	}

	bool AssetManager::DeleteScript(AssetHandle scriptHandle)
	{
		if (!s_ScriptRegistry.contains(scriptHandle))
		{
			KG_WARN("Failed to delete script in AssetManager");
			return false;
		}

		// Delete Handle inside associated class
		for (auto& [classHandle, asset] : s_EntityClassRegistry)
		{
			Ref<Scenes::EntityClass> entityClass = GetEntityClass(classHandle);
			if (!entityClass)
			{
				continue;
			}
			if (entityClass->GetScripts().AllClassScripts.contains(scriptHandle))
			{
				entityClass->GetScripts().AllClassScripts.erase(scriptHandle);
				SaveEntityClass(classHandle, entityClass);
			}
		}

		Utility::FileSystem::DeleteSelectedFile(Projects::ProjectService::GetActiveAssetDirectory() /
			s_ScriptRegistry.at(scriptHandle).Data.IntermediateLocation);

		s_ScriptRegistry.erase(scriptHandle);
		if (s_Scripts.contains(scriptHandle))
		{
			s_Scripts.erase(scriptHandle);
		}

		SerializeScriptRegistry();
		return true;
	}

	bool AssetManager::AddScriptSectionLabel(const std::string& newLabel)
	{
		if (newLabel == "None")
		{
			KG_WARN("Failed to add section label. Cannot add None Label");
			return false;
		}
		if (s_ScriptSectionLabels.contains(newLabel))
		{
			KG_WARN("Failed to add section label. Label already exists in registry");
			return false;
		}

		s_ScriptSectionLabels.insert(newLabel);
		SerializeScriptRegistry();

		return true;
	}

	bool AssetManager::EditScriptSectionLabel(const std::string& oldLabel, const std::string& newLabel)
	{
		if (!s_ScriptSectionLabels.contains(oldLabel))
		{
			KG_WARN("Failed to delete old section label. Label does not exist in registry");
			return false;
		}

		if (s_ScriptSectionLabels.contains(newLabel))
		{
			KG_WARN("Failed to add new section label. Label already exists in registry");
			return false;
		}

		s_ScriptSectionLabels.erase(oldLabel);
		s_ScriptSectionLabels.insert(newLabel);

		// Change label for all scripts
		for (auto& [handle, script] : s_Scripts)
		{
			if (script->m_SectionLabel == oldLabel)
			{
				script->m_SectionLabel = newLabel;
			}
		}

		for (auto& [handle, asset] : s_ScriptRegistry)
		{
			if (asset.Data.GetSpecificFileData<ScriptMetaData>()->SectionLabel == oldLabel)
			{
				asset.Data.GetSpecificFileData<ScriptMetaData>()->SectionLabel = newLabel;
			}
		}

		SerializeScriptRegistry();
		return true;
	}

	bool AssetManager::DeleteScriptSectionLabel(const std::string& label)
	{
		if (!s_ScriptSectionLabels.contains(label))
		{
			KG_WARN("Failed to delete section label. Label does not exist in registry");
			return false;
		}

		s_ScriptSectionLabels.erase(label);

		// Remove this label from all scripts
		for (auto& [handle, script] : s_Scripts)
		{
			if (script->m_SectionLabel == label)
			{
				script->m_SectionLabel = "None";
			}
		}

		for (auto& [handle, asset] : s_ScriptRegistry)
		{
			if (asset.Data.GetSpecificFileData<ScriptMetaData>()->SectionLabel == label)
			{
				asset.Data.GetSpecificFileData<ScriptMetaData>()->SectionLabel = "None";
			}
		}

		SerializeScriptRegistry();

		return true;
	}

	Ref<Scripting::Script> AssetManager::InstantiateScriptIntoMemory(Assets::Asset& asset)
	{
		Assets::ScriptMetaData metadata = *static_cast<Assets::ScriptMetaData*>(asset.Data.SpecificFileData.get());
		Ref<Scripting::Script> newScript = CreateRef<Scripting::Script>();

		newScript->m_ID = asset.Handle;
		newScript->m_ScriptName = metadata.Name;
		newScript->m_FuncType = metadata.FunctionType;
		newScript->m_ScriptType = metadata.ScriptType;
		newScript->m_SectionLabel = metadata.SectionLabel;
		Scripting::ScriptService::LoadScriptFunction(newScript, metadata.FunctionType);

		return newScript;
	}

	Ref<Scripting::Script> AssetManager::GetScript(const AssetHandle& handle)
	{
		KG_ASSERT(Projects::ProjectService::GetActive(), "There is no active project when retreiving Script!");

		if (s_Scripts.contains(handle)) { return s_Scripts[handle]; }

		if (s_ScriptRegistry.contains(handle))
		{
			auto asset = s_ScriptRegistry[handle];

			Ref<Scripting::Script> newScript = InstantiateScriptIntoMemory(asset);
			s_Scripts.insert({ asset.Handle, newScript });
			return newScript;
		}

		KG_ERROR("No Script is associated with provided handle!");
		return nullptr;
	}

	std::tuple<AssetHandle, Ref<Scripting::Script>> AssetManager::GetScript(const std::filesystem::path& filepath)
	{
		KG_ASSERT(Projects::ProjectService::GetActive(), "Attempt to use Project Field without active project!");

		std::filesystem::path ScriptPath = filepath;

		if (filepath.is_absolute())
		{
			ScriptPath = Utility::FileSystem::GetRelativePath(Projects::ProjectService::GetActiveAssetDirectory(), filepath);
		}

		for (auto& [assetHandle, asset] : s_ScriptRegistry)
		{
			if (asset.Data.IntermediateLocation.compare(ScriptPath) == 0)
			{
				if (s_Scripts.contains(assetHandle))
				{
					return std::make_tuple(assetHandle, s_Scripts.at(assetHandle));
				}
				else
				{
					s_Scripts.insert_or_assign(assetHandle, InstantiateScriptIntoMemory(asset));
					return std::make_tuple(assetHandle, s_Scripts.at(assetHandle));
				}
			}
		}
		// Return empty Script if Script does not exist
		KG_WARN("No Script Associated with provided handle. Returned empty Script");
		return std::make_tuple(0, nullptr);
	}

	void AssetManager::ClearScriptRegistry()
	{
		s_ScriptRegistry.clear();
		s_Scripts.clear();
	}

	void AssetManager::FillScriptMetadata(ScriptSpec& spec, Assets::Asset& newAsset)
	{
		// Create script file
		std::string intermediatePath = "Scripting/" + spec.Name + ".kgscript";
		std::filesystem::path intermediateFullPath = Projects::ProjectService::GetActiveAssetDirectory() / intermediatePath;

		Utility::FileSystem::WriteFileString(intermediateFullPath, Utility::GenerateFunctionStub(spec.FunctionType, spec.Name));

		// Load data into In-Memory Metadata object
		newAsset.Data.Type = Assets::AssetType::Script;
		newAsset.Data.IntermediateLocation = intermediatePath;
		Ref<Assets::ScriptMetaData> metadata = CreateRef<Assets::ScriptMetaData>();
		metadata->Name = spec.Name;
		metadata->ScriptType = spec.Type;
		metadata->SectionLabel = spec.SectionLabel;
		metadata->FunctionType = spec.FunctionType;
		newAsset.Data.SpecificFileData = metadata;
	}
}
