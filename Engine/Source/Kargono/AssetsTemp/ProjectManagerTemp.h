#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class ProjectManager : public AssetManagerTemp<Projects::Project>
	{
	public:
		ProjectManager() : AssetManagerTemp<Projects::Project>()
		{
			m_Flags.set(AssetManagerOptions::UseAssetCache, false);
		}
		virtual ~ProjectManager() = default;
	public:
		// Class specific functions
		virtual Ref<Projects::Project> InstantiateAssetIntoMemory(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
	};
}
