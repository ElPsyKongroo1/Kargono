#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class ProjectManager : public AssetManagerTemp<Projects::Project>
	{
	public:
		ProjectManager() : AssetManagerTemp<Projects::Project>()
		{
			m_AssetName = "Project";
			m_FileExtension = ".kproj";
			m_Flags.set(AssetManagerOptions::HasAssetCache, false);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, false);
			m_Flags.set(AssetManagerOptions::HasFileLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileImporting, false);
		}
		virtual ~ProjectManager() = default;
	public:
		// Class specific functions
		virtual Ref<Projects::Project> InstantiateAssetIntoMemory(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
	};
}
