#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class ScriptManager : public AssetManagerTemp<Scripting::Script>
	{
	public:
		ScriptManager() : AssetManagerTemp<Scripting::Script>()
		{
			m_Flags.set(AssetManagerOptions::UseAssetCache, true);
			m_Flags.set(AssetManagerOptions::CreateAssetIntermediate, false);
		}
		virtual ~ScriptManager() = default;
	public:

		// Class specific functions
		virtual Ref<Scripting::Script> InstantiateAssetIntoMemory(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
	};
}
