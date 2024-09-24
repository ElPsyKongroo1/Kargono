#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class ScriptManager : public AssetManagerTemp<Scripting::Script>
	{
	public:
		ScriptManager() = default;
		virtual ~ScriptManager() = default;
	public:
		// Override virtual functions
		virtual Ref<Scripting::Script> InstantiateAssetIntoMemory(Assets::Asset& asset) override;
	};
}
