#include "kgpch.h"

#include "Modules/Scripting/Assets/Script.h"
#include "Modules/Core/Engine.h"
#include "Modules/Events/AssetEvent.h"

namespace Kargono::Scripting
{
	void ScriptRegistryData::Deserialize(void* context)
	{
		// Get asset context
		KG_ASSERT(context, "Context cannot be null");
		Assets::DeserializeRegistryContext& registryContext = *(Assets::DeserializeRegistryContext*)context;

		// Get context fields
		YAML::Node& registryNode = *registryContext.m_RegistryNode;

		// Load in Engine Scripts
		for (Ref<Scripting::Script> script : Scripting::ScriptBinderService::GetActiveContext().GetAllEngineScripts())
		{
			Assets::Metadata newMetadata{};
			newMetadata.m_Handle = script->m_ID;

			// TODO: Access script service/context and set default metadata settings please??

			// Insert scriptMetaData
			ScriptMetaData scriptMetaData{};

			std::string Name{};
			std::vector<WrappedVarType> Parameters{};

			newMetadata.m_Name = script->m_ScriptName;
			scriptMetaData.m_SectionLabel = script->m_SectionLabel;
			scriptMetaData.m_ScriptType = script->m_ScriptType;
			scriptMetaData.m_FunctionType = script->m_FuncType;
			scriptMetaData.m_ExplicitFuncType = script->m_ExplicitFuncType;

			newMetadata.SetSpecificMetaData<ScriptMetaData>(&scriptMetaData);

			// TODO: Access script service/context and insert the engine script yeah??

			// Insert Engine Script into registry/in-memory
			//m_AssetRegistry.insert({ newMetadata.m_Handle, newMetadata });
			//m_AssetCache.insert({ newMetadata.m_Handle, script });
		}

		// Get Section Labels
		{
			m_ScriptSectionLabels.clear();
			YAML::Node sectionLabels = registryNode["SectionLabels"];
			if (sectionLabels)
			{
				for (YAML::Node label : sectionLabels)
				{
					m_ScriptSectionLabels.insert(label.as<std::string>().c_str());
				}
			}
		}
	}

	void ScriptRegistryData::Serialize(void* context)
	{
		// Get asset context
		KG_ASSERT(context, "Context cannot be null");
		Assets::SerializeRegistryContext& metadataContext = *(Assets::SerializeRegistryContext*)context;

		// Get context fields
		KG_ASSERT(metadataContext.m_Serializer);
		YAML::Emitter& serializer = *metadataContext.m_Serializer;

		// Section Labels
		serializer << YAML::Key << "SectionLabels" << YAML::Value;
		serializer << YAML::BeginSeq; // Start SectionLabels

		for (FixedBufStr32 section : m_ScriptSectionLabels)
		{
			serializer << YAML::Value << section.CString(); // Section Name
		}
		serializer << YAML::EndSeq; // Start SectionLabels
	}

	bool ScriptRegistryData::AddScriptSectionLabel(std::string_view newLabel)
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

	bool ScriptRegistryData::EditScriptSectionLabel(std::string_view oldLabel, std::string_view newLabel)
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
			if (asset.Data.GetSpecificMetaData<ScriptMetaData>()->m_SectionLabel == oldLabel)
			{
				asset.Data.GetSpecificMetaData<ScriptMetaData>()->m_SectionLabel = newLabel;
			}
		}

		SerializeAssetRegistry();

		Ref<Events::ManageAsset> event = CreateRef<Events::ManageAsset>
			(
				Assets::k_EmptyHandle,
				Assets::GetAssetIdentifier<Script>(),
				Events::ManageAssetAction::UpdateAsset
			);
		EngineService::GetActiveEngine().GetThread().SubmitEvent(event);
		return true;
	}

	bool ScriptRegistryData::DeleteScriptSectionLabel(std::string_view label)
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
			if (asset.Data.GetSpecificMetaData<ScriptMetaData>()->m_SectionLabel == label)
			{
				asset.Data.GetSpecificMetaData<ScriptMetaData>()->m_SectionLabel = "None";
			}
		}

		SerializeAssetRegistry();

		Ref<Events::ManageAsset> event = CreateRef<Events::ManageAsset>
		(
			Assets::k_EmptyHandle,
			Assets::GetAssetIdentifier<Script>(),
			Events::ManageAssetAction::UpdateAsset
		);
		EngineService::GetActiveEngine().GetThread().SubmitEvent(event);
		return true;
	}

	void ScriptMetaData::Deserialize(void* context)
	{
		// Get asset context
		KG_ASSERT(context, "Context cannot be null");
		Assets::DeserializeMetaDataContext& assetContext = *(Assets::DeserializeMetaDataContext*)context;

		// Get context fields
		YAML::Node& metadataNode = *assetContext.m_Node;

		// Deserialize
		m_SectionLabel = metadataNode["SectionLabel"].as<std::string>();
		m_ScriptType = Utility::StringToScriptType(metadataNode["ScriptType"].as<std::string>());
		m_FunctionType = Utility::StringToWrappedFuncType(metadataNode["FunctionType"].as<std::string>());
		if (m_FunctionType == WrappedFuncType::ArbitraryFunction)
		{
			// Deserialize explicit func type
			Scripting::ExplicitFuncType& explicitFuncType = m_ExplicitFuncType;
			explicitFuncType = {};
			YAML::Node explicitNode = metadataNode["ExplicitFuncType"];
			KG_ASSERT(explicitNode);

			// Deserialize return type
			explicitFuncType.m_ReturnType = Utility::StringToWrappedVarType(explicitNode["ReturnType"].as<std::string>());

			// Deserialize parameter types
			YAML::Node parameterTypes = explicitNode["ParameterTypes"];
			KG_ASSERT(parameterTypes);
			for (YAML::detail::iterator_value parameterType : parameterTypes)
			{
				explicitFuncType.m_ParameterTypes.push_back(Utility::StringToWrappedVarType(parameterType.as<std::string>()));
			}

			// Deserialize parameter names
			YAML::Node parameterNames = explicitNode["ParameterNames"];
			KG_ASSERT(parameterNames);
			for (YAML::detail::iterator_value parameterName : parameterNames)
			{
				explicitFuncType.m_ParameterNames.push_back(parameterName.as<std::string>().c_str());
			}
		}
	}

	void ScriptMetaData::Serialize(void* context)
	{
		// Get asset context
		KG_ASSERT(context, "Context cannot be null");
		Assets::SerializeMetaDataContext& metadataContext = *(Assets::SerializeMetaDataContext*)context;

		// Get context fields
		KG_ASSERT(metadataContext.m_Serializer);
		YAML::Emitter& serializer = *metadataContext.m_Serializer;
		KG_ASSERT(metadataContext.m_Metadata);
		Assets::Metadata& metadata = *metadataContext.m_Metadata;

		// Serialize
		serializer << YAML::Key << "SectionLabel" << YAML::Value << m_SectionLabel;
		serializer << YAML::Key << "ScriptType" << YAML::Value << Utility::ScriptTypeToString(m_ScriptType);
		serializer << YAML::Key << "FunctionType" << YAML::Value << Utility::WrappedFuncTypeToString(m_FunctionType);

		Scripting::ExplicitFuncType& explicitFuncType = m_ExplicitFuncType;
		if (m_FunctionType == WrappedFuncType::ArbitraryFunction)
		{
			// Serialize explicit func definition
			serializer << YAML::Key << "ExplicitFuncType" << YAML::BeginMap; // Explicit func type map

			// Serialize return type
			serializer << YAML::Key << "ReturnType" << YAML::Value << Utility::WrappedVarTypeToString(explicitFuncType.m_ReturnType);

			// Serialize parameter types
			serializer << YAML::Key << "ParameterTypes" << YAML::Value;
			serializer << YAML::BeginSeq; // Start parameter types list
			for (WrappedVarType type : explicitFuncType.m_ParameterTypes)
			{
				serializer << YAML::Value << Utility::WrappedVarTypeToString(type);
			}
			serializer << YAML::EndSeq; // End parameter types list

			// Serialize parameter names
			serializer << YAML::Key << "ParameterNames" << YAML::Value;
			serializer << YAML::BeginSeq; // Start parameter types list
			for (FixedBufStr32& name : explicitFuncType.m_ParameterNames)
			{
				serializer << YAML::Value << name.CString();
			}
			serializer << YAML::EndSeq; // End parameter types list

			serializer << YAML::EndMap; // Explicit func type map
		}
	}

	void Script::Serialize(void* context)
	{
		// Get asset context
		Assets::SerializeAssetContext* assetContext = (Assets::SerializeAssetContext*)context;
		KG_ASSERT(assetContext, "Context cannot be null");
		Assets::Metadata* metadata{ assetContext->m_AssetMetadata };
		KG_ASSERT(metadata, "Metadata cannot be null");

		// Get asset path
		const std::filesystem::path& assetPath{ metadata->GetAssetFullFilePath<Script>() };

		KG_ERROR("We got some splaining to dooooooo");

	}

	void Script::Deserialize(void* context)
	{
		KG_ASSERT(context, "Context cannot be null");

		// Get asset context
		Assets::DeserializeAssetContext& assetContext = *(Assets::DeserializeAssetContext*)context;
		KG_ASSERT(assetContext.m_AssetMetadata, "Metadata cannot be null");
		Assets::Metadata* metadata{ assetContext.m_AssetMetadata };
		KG_ASSERT(metadata, "Metadata cannot be null");

		// Get asset path
		const std::filesystem::path& assetPath{ metadata->GetAssetFullFilePath<Script>() };

		ScriptMetaData& scriptMetadata = *metadata->GetSpecificMetaData<ScriptMetaData>();

		m_ID = metadata->m_Handle;
		m_ScriptName = metadata->m_Name;
		m_FuncType = scriptMetadata.m_FunctionType;
		m_ScriptType = scriptMetadata.m_ScriptType;
		m_SectionLabel = scriptMetadata.m_SectionLabel;
		m_ExplicitFuncType = scriptMetadata.m_ExplicitFuncType;
		Scripting::ScriptBinderService::GetActiveContext().LoadScriptFunction(this, scriptMetadata.m_FunctionType);
	}

	void Script::CreateFromSpec(Assets::Metadata& metadata, const ScriptSpec& spec)
	{
		const std::filesystem::path fullPath = metadata.GetAssetFullFilePath<Script>();

		// Create the script file on-disk
		Utility::FileSystem::WriteFileString(fullPath, Utility::GenerateFunctionStub(spec.m_FunctionType, 
			metadata.m_Name.StringView(), spec.m_ExplicitFuncType));

		// Load data into in-memory metadata object
		ScriptMetaData* scriptMetadata = metadata.GetSpecificMetaData<ScriptMetaData>();
		KG_ASSERT(scriptMetadata);
		scriptMetadata->m_ScriptType = spec.m_Type;
		scriptMetadata->m_SectionLabel = spec.m_SectionLabel;
		scriptMetadata->m_FunctionType = spec.m_FunctionType;
		scriptMetadata->m_ExplicitFuncType = spec.m_ExplicitFuncType;
	}

	bool Script::ValidateCreateFromSpec(const Assets::AssetCreationData& creationData, const ScriptSpec& spec)
	{
		// Check if function type is valid
		if (spec.m_FunctionType == WrappedFuncType::None)
		{
			KG_WARN("Unable to create new script. Invalid Function Type Provided!");
			return false;
		}
		return true;
	}

	void Script::ValidateDelete(Assets::Metadata& metadata)
	{
		// Ensure all other assets do not contain this script
		// If they do, remove the reference

		// Check StateMachines State assets
		for (auto& [aiHandle, assetInfo] : Assets::s_StateManager.GetAssetRegistry())
		{
		    Assets::AssetRef<States::State> aiStateRef = Assets::s_StateManager.GetAssetByHandle(aiHandle);
			bool aiModified = AssetService::RemoveScriptFromState(aiStateRef, metadata.m_Handle);

			if (aiModified)
			{
				Assets::s_StateManager.UpdateAsset(aiStateRef);
			}
		}

		// Check input maps assets
		for (auto& [inputHandle, assetInfo] : Assets::s_InputMapManager.GetAssetRegistry())
		{
		    Assets::AssetRef<Input::InputMap> inputMapRef = Assets::s_InputMapManager.GetAssetByHandle(inputHandle);
			bool inputModified = inputMapRef->RemoveScript(metadata.m_Handle);

			if (inputModified)
			{
				Assets::s_InputMapManager.UpdateAsset(inputMapRef);
			}
		}

		// Check user interface assets
		for (auto& [uiHandle, assetInfo] : Assets::s_UserInterfaceManager.GetAssetRegistry())
		{
			// Handle UI level function pointers
		    Assets::AssetRef<RuntimeUI::UserInterface> userInterfaceRef = Assets::s_UserInterfaceManager.GetAssetByHandle(uiHandle);
			bool uiModified = userInterfaceRef->RemoveScript(metadata.m_Handle);
			if (uiModified)
			{
				Assets::s_UserInterfaceManager.UpdateAsset(userInterfaceRef);
			}
		}

		// Check scene assets
		for (auto& [sceneHandle, assetInfo] : Assets::s_SceneManager.GetAssetRegistry())
		{
			// Handle UI level function pointers
		    Assets::AssetRef<Scenes::Scene> sceneRef = Assets::s_SceneManager.GetAssetByHandle(sceneHandle);

			bool sceneModified = sceneRef->RemoveScript(metadata.m_Handle);
			if (sceneModified)
			{
				// Save scene
				Assets::s_SceneManager.UpdateAsset(sceneRef);
			}
		}

		Projects::Project& activeProject{ Projects::ProjectService::GetActiveContext() };

		// Check active project for scripts
		bool projectModified = activeProject.RemoveScriptFromActiveProject(metadata.m_Handle);
		if (projectModified)
		{
			activeProject.SaveProject();
		}
	}

	void Script::UpdateFromSpec(Assets::Metadata& metadata, const ScriptSpec& spec)
	{
		ScriptMetaData* scriptMetadata = metadata.GetSpecificMetaData<ScriptMetaData>();
		KG_ASSERT(scriptMetadata);

		// Update script metadata
		scriptMetadata->m_ScriptType = spec.m_Type;
		scriptMetadata->m_SectionLabel = spec.m_SectionLabel;

		// Update fields
		m_ScriptType = spec.m_Type;
		m_SectionLabel = spec.m_SectionLabel;
		m_Function = nullptr;
	}

}