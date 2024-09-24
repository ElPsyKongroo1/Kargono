#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class SceneManager : public AssetManagerTemp<Scenes::Scene>
	{
	public:
		SceneManager() : AssetManagerTemp<Scenes::Scene>()
		{
			m_Flags.set(AssetManagerOptions::UseRuntimeCache, false);
		}
		virtual ~SceneManager() = default;
	public:
		// Override virtual functions
		virtual Ref<Scenes::Scene> InstantiateAssetIntoMemory(Assets::Asset& asset) override;

		// Class specific functions
		bool DeserializeScene(Ref<Scenes::Scene> Scene, const std::filesystem::path& filepath);
	};
}
