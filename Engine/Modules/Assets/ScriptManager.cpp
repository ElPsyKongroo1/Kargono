#include "kgpch.h"

#include "Modules/Assets/AssetService.h"
#include "Modules/Assets/ScriptManager.h"
#include "Kargono/Utility/Regex.h"
#include "Modules/Scripting/ScriptModuleBinder.h"
#include "Modules/AI/AIContext.h"
#include "Modules/InputMap/Assets/InputMap.h"
#include "Modules/RuntimeUI/RuntimeUIContext.h"
#include "Kargono/Scenes/Scene.h"
#include "Modules/ECS/Entity.h"
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
		AssetHandle newHandle{ RandomUUIDService::GetRandomUUID() };
		Assets::AssetInfo newAsset{};
		newAsset.Data.Type = AssetType::Script;
		newAsset.Data.FileLocation = spec.Name + m_FileExtension.CString();
		newAsset.Data.CheckSum = currentCheckSum;
		newAsset.m_Handle = newHandle;

		// Create Script File
		FillScriptMetadata(spec, newAsset);
		newAsset.Data.CheckSum = currentCheckSum;

		// Register New Asset and Create Script
		m_AssetRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeAssetRegistry(); // Update Registry File on Disk

		Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };

		m_AssetCache.insert({ newHandle, DeserializeAsset(newAsset, projectPaths.GetAssetDirectory() / newAsset.Data.FileLocation) });

		Ref<Events::ManageAsset> event = CreateRef<Events::ManageAsset>
		(
			newHandle,
			newAsset.Data.Type, 
			Events::ManageAssetAction::Create
		);
		EngineService::GetActiveEngine().GetThread().SubmitEvent(event);
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

		Ref<Events::ManageAsset> event = CreateRef<Events::ManageAsset>
		(
			scriptHandle, 
			asset.Data.Type, 
			Events::ManageAssetAction::UpdateAsset
		);
		EngineService::GetActiveEngine().GetThread().SubmitEvent(event);
		return true;
	}

	void ScriptManager::FillScriptMetadata(ScriptSpec& spec, Assets::AssetInfo& newAsset)
	{

		// Create script file
		Projects::ProjectPaths& projectPaths{ Projects::ProjectService::GetActiveContext().GetProjectPaths() };
		std::filesystem::path fullPath = projectPaths.GetAssetDirectory() / newAsset.Data.FileLocation;

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
}
