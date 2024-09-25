#include "kgpch.h"

#include "Kargono/AssetsTemp/AssetService.h"
#include "Kargono/AssetsTemp/ProjectManagerTemp.h"
#include "Kargono/Projects/Project.h"

namespace Kargono::Assets
{
	Ref<Projects::Project> ProjectManager::InstantiateAssetIntoMemory(Assets::Asset& asset, const std::filesystem::path& assetPath)
	{
		return Ref<Projects::Project>();
	}
}
