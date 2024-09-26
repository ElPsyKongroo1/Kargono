#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class SceneManager : public AssetManagerTemp<Scenes::Scene>
	{
	public:
		SceneManager() : AssetManagerTemp<Scenes::Scene>()
		{
			m_AssetName = "Scene";
			m_FileExtension = ".kgscene";
			m_Flags.set(AssetManagerOptions::HasAssetCache, false);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, false);
			m_Flags.set(AssetManagerOptions::HasFileLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileImporting, false);
		}
		virtual ~SceneManager() = default;
	public:

		// Class specific functions
		virtual Ref<Scenes::Scene> InstantiateAssetIntoMemory(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
	};
}
