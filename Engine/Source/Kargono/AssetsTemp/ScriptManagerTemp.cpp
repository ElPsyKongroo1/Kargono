#include "kgpch.h"

#include "Kargono/AssetsTemp/AssetService.h"
#include "Kargono/AssetsTemp/ScriptManagerTemp.h"
#include "Kargono/Utility/Regex.h"
#include "Kargono/Scripting/ScriptService.h"


namespace Kargono::Assets
{
	Ref<Scripting::Script> ScriptManager::InstantiateAssetIntoMemory(Assets::Asset& asset, const std::filesystem::path& assetPath)
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
}
