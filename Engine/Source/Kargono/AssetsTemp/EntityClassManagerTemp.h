#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class EntityClassManager : public AssetManagerTemp<Scenes::EntityClass>
	{
	public:
		EntityClassManager() : AssetManagerTemp<Scenes::EntityClass>()
		{
			m_Flags.set(AssetManagerOptions::UseRuntimeCache, false);
		}
		virtual ~EntityClassManager() = default;
	public:
		// Override virtual functions
		virtual Ref<Scenes::EntityClass> InstantiateAssetIntoMemory(Assets::Asset& asset) override;

		// Class specific functions
		bool DeserializeEntityClass(Ref<Scenes::EntityClass> EntityClass, const std::filesystem::path& filepath);
	};
}
