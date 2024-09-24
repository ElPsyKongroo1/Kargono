#include "kgpch.h"

#include "Kargono/AssetsTemp/AssetService.h"
#include "Kargono/AssetsTemp/ScriptManagerTemp.h"
#include "Kargono/Utility/Regex.h"
#include "Kargono/Scripting/ScriptService.h"


namespace Kargono::Assets
{

	Ref<Scripting::Script> ScriptManager::InstantiateAssetIntoMemory(Assets::Asset& asset)
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

	static ScriptManager s_ScriptManager;

	Ref<Scripting::Script> AssetServiceTemp::GetScript(const AssetHandle& handle)
	{
		return s_ScriptManager.GetAsset(handle);
	}

	std::filesystem::path Kargono::Assets::AssetServiceTemp::GetScriptIntermediateLocation(const AssetHandle& handle)
	{
		return s_ScriptManager.GetAssetIntermediateLocation(handle);
	}
}
