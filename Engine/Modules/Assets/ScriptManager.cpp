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
}
