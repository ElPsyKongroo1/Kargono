#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class ScriptManager : public AssetManagerTemp<Scripting::Script>
	{
	public:
		ScriptManager() : AssetManagerTemp<Scripting::Script>()
		{
			m_AssetName = "Script";
			m_FileExtension = ".kgscript";
			m_Flags.set(AssetManagerOptions::HasAssetCache, true);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, false);
			m_Flags.set(AssetManagerOptions::HasFileLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileImporting, false);
		}
		virtual ~ScriptManager() = default;
	public:

		// Class specific functions
		virtual Ref<Scripting::Script> InstantiateAssetIntoMemory(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
	};
}
