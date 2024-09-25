#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class EntityClassManager : public AssetManagerTemp<Scenes::EntityClass>
	{
	public:
		EntityClassManager() : AssetManagerTemp<Scenes::EntityClass>()
		{
			m_Flags.set(AssetManagerOptions::UseAssetCache, false);
		}
		virtual ~EntityClassManager() = default;
	public:

		// Class specific functions
		virtual Ref<Scenes::EntityClass> InstantiateAssetIntoMemory(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
	};
}
