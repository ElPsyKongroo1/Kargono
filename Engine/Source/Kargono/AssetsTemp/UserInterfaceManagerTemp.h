#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class UserInterfaceManager : public AssetManagerTemp<RuntimeUI::UserInterface>
	{
	public:
		UserInterfaceManager() : AssetManagerTemp<RuntimeUI::UserInterface>()
		{
			m_Flags.set(AssetManagerOptions::UseAssetCache, false);
			m_Flags.set(AssetManagerOptions::CreateAssetIntermediate, false);
		}
		virtual ~UserInterfaceManager() = default;
	public:
		// Class specific functions
		virtual Ref<RuntimeUI::UserInterface> InstantiateAssetIntoMemory(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
	};
}
