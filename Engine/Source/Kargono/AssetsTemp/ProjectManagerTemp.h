#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class ProjectManager : public AssetManagerTemp<Projects::Project>
	{
	public:
		ProjectManager() : AssetManagerTemp<Projects::Project>()
		{
			m_Flags.set(AssetManagerOptions::UseRuntimeCache, false);
		}
		virtual ~ProjectManager() = default;
	public:
		// Override virtual functions
		virtual Ref<Projects::Project> InstantiateAssetIntoMemory(Assets::Asset& asset) override { return nullptr; };
	};
}
