#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class SceneManager : public AssetManagerTemp<Scenes::Scene>
	{
	public:
		SceneManager() : AssetManagerTemp<Scenes::Scene>()
		{
			m_Flags.set(AssetManagerOptions::UseAssetCache, false);
		}
		virtual ~SceneManager() = default;
	public:

		// Class specific functions
		virtual Ref<Scenes::Scene> InstantiateAssetIntoMemory(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
	};
}
