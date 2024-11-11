#include "kgpch.h"

#include "Kargono/Assets/AssetService.h"
#include "Kargono/Assets/ScriptManager.h"
#include "Kargono/Utility/Regex.h"
#include "Kargono/Scripting/ScriptService.h"
#include "Kargono/AI/AIService.h"
#include "Kargono/Input/InputMap.h"
#include "Kargono/RuntimeUI/RuntimeUI.h"
#include "Kargono/Scenes/Scene.h"
#include "Kargono/ECS/Entity.h"
#include "Kargono/Projects/Project.h"

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

		// Create Checksum
		const std::string currentCheckSum {};

		// Create New Asset/Handle
		AssetHandle newHandle{};
		Assets::AssetInfo newAsset{};
		newAsset.Data.Type = AssetType::Script;
		newAsset.Data.FileLocation = spec.Name + m_FileExtension.CString();
		newAsset.Data.CheckSum = currentCheckSum;
		newAsset.Handle = newHandle;

		// Create Script File
		FillScriptMetadata(spec, newAsset);
		newAsset.Data.CheckSum = currentCheckSum;

		// Register New Asset and Create Script
		m_AssetRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeAssetRegistry(); // Update Registry File on Disk

		m_AssetCache.insert({ newHandle, DeserializeAsset(newAsset, Projects::ProjectService::GetActiveAssetDirectory() / newAsset.Data.FileLocation) });

		Ref<Events::ManageAsset> event = CreateRef<Events::ManageAsset>(newHandle, newAsset.Data.Type, Events::ManageAssetAction::Create);
		EngineService::SubmitToEventQueue(event);
		return std::make_tuple(newHandle, true);
	}

	bool ScriptManager::SaveScript(AssetHandle scriptHandle, ScriptSpec& spec)
	{
		// Get original asset/metadata
		AssetInfo asset = m_AssetRegistry.at(scriptHandle);
		ScriptMetaData* metadata = asset.Data.GetSpecificMetaData<ScriptMetaData>();

		// Check if script exists in registry
		if (!m_AssetRegistry.contains(scriptHandle))
		{
			KG_WARN("Unable to update script. Does not exist in registry.");
			return false;
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

		Ref<Events::ManageAsset> event = CreateRef<Events::ManageAsset>(scriptHandle, asset.Data.Type, Events::ManageAssetAction::UpdateAsset);
		EngineService::SubmitToEventQueue(event);
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

		Ref<Events::ManageAsset> event = CreateRef<Events::ManageAsset>(Assets::EmptyHandle, AssetType::Script, Events::ManageAssetAction::UpdateAsset);
		EngineService::SubmitToEventQueue(event);
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

		Ref<Events::ManageAsset> event = CreateRef<Events::ManageAsset>(Assets::EmptyHandle, AssetType::Script, Events::ManageAssetAction::UpdateAsset);
		EngineService::SubmitToEventQueue(event); 
		return true;
	}

	void ScriptManager::FillScriptMetadata(ScriptSpec& spec, Assets::AssetInfo& newAsset)
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
	Ref<Scripting::Script> ScriptManager::DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath)
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
		serializer << YAML::EndSeq; // Start SectionLabels
	}
	void ScriptManager::SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::AssetInfo& currentAsset)
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
			Assets::AssetInfo newAsset{};
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
	void ScriptManager::DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::AssetInfo& currentAsset)
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
		// Ensure all other assets do not contain this script
		// If they do, remove the reference

		// Check AI State assets
		for (auto& [aiHandle, assetInfo] : Assets::AssetService::GetAIStateRegistry())
		{
			Ref<AI::AIState> aiStateRef = Assets::AssetService::GetAIState(aiHandle);
			if (aiStateRef->OnEnterStateHandle == scriptHandle)
			{
				aiStateRef->OnEnterStateHandle = Assets::EmptyHandle;
				aiStateRef->OnEnterState = nullptr;
			}

			if (aiStateRef->OnExitStateHandle == scriptHandle)
			{
				aiStateRef->OnExitStateHandle = Assets::EmptyHandle;
				aiStateRef->OnExitState = nullptr;
			}

			if (aiStateRef->OnUpdateHandle == scriptHandle)
			{
				aiStateRef->OnUpdateHandle = Assets::EmptyHandle;
				aiStateRef->OnUpdate = nullptr;
			}

			if (aiStateRef->OnMessageHandle == scriptHandle)
			{
				aiStateRef->OnMessageHandle = Assets::EmptyHandle;
				aiStateRef->OnMessage = nullptr;
			}

			Assets::AssetService::SaveAIState(aiHandle, aiStateRef);
		}

		// Check input maps assets
		for (auto& [inputHandle, assetInfo] : Assets::AssetService::GetInputMapRegistry())
		{
			Ref<Input::InputMap> inputMapRef = Assets::AssetService::GetInputMap(inputHandle);
			
			for (Ref<Input::InputActionBinding> binding : inputMapRef->GetOnUpdateBindings())
			{
				if (binding->GetScriptHandle() == scriptHandle)
				{
					binding->SetScript(nullptr, Assets::EmptyHandle);
				}
			}

			for (Ref<Input::InputActionBinding> binding : inputMapRef->GetOnKeyPressedBindings())
			{
				if (binding->GetScriptHandle() == scriptHandle)
				{
					binding->SetScript(nullptr, Assets::EmptyHandle);
				}
			}

			Assets::AssetService::SaveInputMap(inputHandle, inputMapRef);
		}

		// Check user interface assets
		for (auto& [uiHandle, assetInfo] : Assets::AssetService::GetUserInterfaceRegistry())
		{
			// Handle UI level function pointers
			Ref<RuntimeUI::UserInterface> userInterfaceRef = Assets::AssetService::GetUserInterface(uiHandle);
			if (userInterfaceRef->m_FunctionPointers.OnMoveHandle == scriptHandle)
			{
				userInterfaceRef->m_FunctionPointers.OnMoveHandle = Assets::EmptyHandle;
				userInterfaceRef->m_FunctionPointers.OnMove = nullptr;
			}

			// Handle all widgets
			for (RuntimeUI::Window& currentWindow : userInterfaceRef->m_Windows)
			{
				for (Ref<RuntimeUI::Widget> widgetRef : currentWindow.Widgets)
				{
					if (widgetRef->FunctionPointers.OnPressHandle == scriptHandle)
					{
						widgetRef->FunctionPointers.OnPressHandle = Assets::EmptyHandle;
						widgetRef->FunctionPointers.OnPress = nullptr;
					}
				}
			}

			Assets::AssetService::SaveUserInterface(uiHandle, userInterfaceRef);
		}

		// Check scene assets
		for (auto& [uiHandle, assetInfo] : Assets::AssetService::GetSceneRegistry())
		{
			// Handle UI level function pointers
			Ref<Scenes::Scene> sceneRef = Assets::AssetService::GetScene(uiHandle);

			// OnUpdate
			auto onUpdateView = sceneRef->GetAllEntitiesWith<ECS::OnUpdateComponent>();
			for (entt::entity enttEntity : onUpdateView)
			{
				ECS::Entity currentEntity{ sceneRef->GetEntityByEnttID(enttEntity) };
				ECS::OnUpdateComponent& component = currentEntity.GetComponent<ECS::OnUpdateComponent>();
				if (component.OnUpdateScriptHandle == scriptHandle)
				{
					component.OnUpdateScriptHandle = Assets::EmptyHandle;
					component.OnUpdateScript = nullptr;
				}
			}

			// OnCreate
			auto onCreateView = sceneRef->GetAllEntitiesWith<ECS::OnCreateComponent>();
			for (entt::entity enttEntity : onCreateView)
			{
				ECS::Entity currentEntity{ sceneRef->GetEntityByEnttID(enttEntity) };
				ECS::OnCreateComponent& component = currentEntity.GetComponent<ECS::OnCreateComponent>();
				if (component.OnCreateScriptHandle == scriptHandle)
				{
					component.OnCreateScriptHandle = Assets::EmptyHandle;
					component.OnCreateScript = nullptr;
				}
			}

			// Rigidbody
			auto rigidBodyView = sceneRef->GetAllEntitiesWith<ECS::Rigidbody2DComponent>();
			for (entt::entity enttEntity : rigidBodyView)
			{
				ECS::Entity currentEntity{ sceneRef->GetEntityByEnttID(enttEntity) };
				ECS::Rigidbody2DComponent& component = currentEntity.GetComponent<ECS::Rigidbody2DComponent>();

				if (component.OnCollisionStartScriptHandle == scriptHandle)
				{
					component.OnCollisionStartScriptHandle = Assets::EmptyHandle;
					component.OnCollisionStartScript = nullptr;
				}

				if (component.OnCollisionEndScriptHandle == scriptHandle)
				{
					component.OnCollisionEndScriptHandle = Assets::EmptyHandle;
					component.OnCollisionEndScript = nullptr;
				}
			}

			// Save scene
			Assets::AssetService::SaveScene(uiHandle, sceneRef);
		}

		// Check active project for scripts
		if (Projects::ProjectService::GetActiveOnRuntimeStartHandle() == scriptHandle)
		{
			Projects::ProjectService::SetActiveOnRuntimeStartHandle(Assets::EmptyHandle);
		}
		if (Projects::ProjectService::GetActiveOnUpdateUserCountHandle() == scriptHandle)
		{
			Projects::ProjectService::SetActiveOnUpdateUserCountHandle(Assets::EmptyHandle);
		}
		if (Projects::ProjectService::GetActiveOnApproveJoinSessionHandle() == scriptHandle)
		{
			Projects::ProjectService::SetActiveOnApproveJoinSessionHandle(Assets::EmptyHandle);
		}
		if (Projects::ProjectService::GetActiveOnUserLeftSessionHandle() == scriptHandle)
		{
			Projects::ProjectService::SetActiveOnUserLeftSessionHandle(Assets::EmptyHandle);
		}
		if (Projects::ProjectService::GetActiveOnCurrentSessionInitHandle() == scriptHandle)
		{
			Projects::ProjectService::SetActiveOnCurrentSessionInitHandle(Assets::EmptyHandle);
		}
		if (Projects::ProjectService::GetActiveOnConnectionTerminatedHandle() == scriptHandle)
		{
			Projects::ProjectService::SetActiveOnConnectionTerminatedHandle(Assets::EmptyHandle);
		}
		if (Projects::ProjectService::GetActiveOnUpdateSessionUserSlotHandle() == scriptHandle)
		{
			Projects::ProjectService::SetActiveOnUpdateSessionUserSlotHandle(Assets::EmptyHandle);
		}
		if (Projects::ProjectService::GetActiveOnStartSessionHandle() == scriptHandle)
		{
			Projects::ProjectService::SetActiveOnStartSessionHandle(Assets::EmptyHandle);
		}
		if (Projects::ProjectService::GetActiveOnSessionReadyCheckConfirmHandle() == scriptHandle)
		{
			Projects::ProjectService::SetActiveOnSessionReadyCheckConfirmHandle(Assets::EmptyHandle);
		}
		if (Projects::ProjectService::GetActiveOnReceiveSignalHandle() == scriptHandle)
		{
			Projects::ProjectService::SetActiveOnReceiveSignalHandle(Assets::EmptyHandle);
		}
	}
}
