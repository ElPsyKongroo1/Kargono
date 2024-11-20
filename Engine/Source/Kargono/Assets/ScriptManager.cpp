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
			if (metadata.m_Name == spec.Name)
			{
				KG_WARN("Unable to create new script. Script Name already exists in asset manager");
				return std::make_tuple(0, false);
			}
		}

		// Check if function type is valid
		if (spec.m_FunctionType == WrappedFuncType::None)
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

		// Update registry metadata
		metadata->m_Name = spec.Name;
		metadata->m_ScriptType = spec.Type;
		metadata->m_SectionLabel = spec.m_SectionLabel;

		SerializeAssetRegistry();

		// Update In-Memory Script
		if (m_AssetCache.contains(scriptHandle))
		{
			Ref<Scripting::Script> script = m_AssetCache.at(scriptHandle);
			script->m_ScriptName = spec.Name;
			script->m_ScriptType = spec.Type;
			script->m_SectionLabel = spec.m_SectionLabel;
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
			if (asset.Data.GetSpecificMetaData<ScriptMetaData>()->m_SectionLabel == oldLabel)
			{
				asset.Data.GetSpecificMetaData<ScriptMetaData>()->m_SectionLabel = newLabel;
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
			if (asset.Data.GetSpecificMetaData<ScriptMetaData>()->m_SectionLabel == label)
			{
				asset.Data.GetSpecificMetaData<ScriptMetaData>()->m_SectionLabel = "None";
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

		Utility::FileSystem::WriteFileString(fullPath, Utility::GenerateFunctionStub(spec.m_FunctionType, spec.Name, spec.m_ExplicitFuncType));

		// Load data into In-Memory Metadata object
		Ref<Assets::ScriptMetaData> metadata = CreateRef<Assets::ScriptMetaData>();
		metadata->m_Name = spec.Name;
		metadata->m_ScriptType = spec.Type;
		metadata->m_SectionLabel = spec.m_SectionLabel;
		metadata->m_FunctionType = spec.m_FunctionType;
		metadata->m_ExplicitFuncType = spec.m_ExplicitFuncType;
		
		newAsset.Data.SpecificFileData = metadata;
	}
	Ref<Scripting::Script> ScriptManager::DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath)
	{
		Ref<Scripting::Script> newScript = CreateRef<Scripting::Script>();
		Assets::ScriptMetaData metadata = *asset.Data.GetSpecificMetaData<ScriptMetaData>();

		newScript->m_ID = asset.Handle;
		newScript->m_ScriptName = metadata.m_Name;
		newScript->m_FuncType = metadata.m_FunctionType;
		newScript->m_ScriptType = metadata.m_ScriptType;
		newScript->m_SectionLabel = metadata.m_SectionLabel;
		newScript->m_ExplicitFuncType = metadata.m_ExplicitFuncType;
		Scripting::ScriptService::LoadScriptFunction(newScript, metadata.m_FunctionType);

		return newScript;
	}
	void ScriptManager::SerializeRegistrySpecificData(YAML::Emitter& serializer)
	{
		// Section Labels
		serializer << YAML::Key << "SectionLabels" << YAML::Value;
		serializer << YAML::BeginSeq; // Start SectionLabels

		for (const std::string& section : m_ScriptSectionLabels)
		{
			serializer << YAML::Value << section; // Section Name
		}
		serializer << YAML::EndSeq; // Start SectionLabels
	}
	void ScriptManager::SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::AssetInfo& currentAsset)
	{
		Assets::ScriptMetaData* metadata = static_cast<Assets::ScriptMetaData*>(currentAsset.Data.SpecificFileData.get());

		serializer << YAML::Key << "Name" << YAML::Value << metadata->m_Name;
		serializer << YAML::Key << "SectionLabel" << YAML::Value << metadata->m_SectionLabel;
		serializer << YAML::Key << "ScriptType" << YAML::Value << Utility::ScriptTypeToString(metadata->m_ScriptType);
		serializer << YAML::Key << "FunctionType" << YAML::Value << Utility::WrappedFuncTypeToString(metadata->m_FunctionType);


		Scripting::ExplicitFuncType& explicitFuncType = metadata->m_ExplicitFuncType;
		if (metadata->m_FunctionType == WrappedFuncType::ArbitraryFunction)
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
			for (FixedString32& name : explicitFuncType.m_ParameterNames)
			{
				serializer << YAML::Value << name.CString();
			}
			serializer << YAML::EndSeq; // End parameter types list

			serializer << YAML::EndMap; // Explicit func type map
		}
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

			// Insert scriptMetaData
			Ref<Assets::ScriptMetaData> scriptMetaData = CreateRef<Assets::ScriptMetaData>();

			std::string Name{};
			std::vector<WrappedVarType> Parameters{};

			scriptMetaData->m_Name = script->m_ScriptName;
			scriptMetaData->m_SectionLabel = script->m_SectionLabel;
			scriptMetaData->m_ScriptType = script->m_ScriptType;
			scriptMetaData->m_FunctionType = script->m_FuncType;
			scriptMetaData->m_ExplicitFuncType = script->m_ExplicitFuncType;
			
			newAsset.Data.SpecificFileData = scriptMetaData;

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
		Ref<Assets::ScriptMetaData> scriptMetaData = CreateRef<Assets::ScriptMetaData>();

		std::string Name{};
		std::vector<WrappedVarType> Parameters{};

		scriptMetaData->m_Name = metadataNode["Name"].as<std::string>();

		scriptMetaData->m_SectionLabel = metadataNode["SectionLabel"].as<std::string>();
		scriptMetaData->m_ScriptType = Utility::StringToScriptType(metadataNode["ScriptType"].as<std::string>());
		scriptMetaData->m_FunctionType = Utility::StringToWrappedFuncType(metadataNode["FunctionType"].as<std::string>());
		if (scriptMetaData->m_FunctionType == WrappedFuncType::ArbitraryFunction)
		{
			// Deserialize explicit func type
			Scripting::ExplicitFuncType& explicitFuncType = scriptMetaData->m_ExplicitFuncType;
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
		currentAsset.Data.SpecificFileData = scriptMetaData;
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
