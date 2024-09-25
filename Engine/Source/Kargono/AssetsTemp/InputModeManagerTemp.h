#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class InputModeManager : public AssetManagerTemp<Input::InputMode>
	{
	public:
		InputModeManager() : AssetManagerTemp<Input::InputMode>()
		{
			m_Flags.set(AssetManagerOptions::UseAssetCache, false);
			m_Flags.set(AssetManagerOptions::CreateAssetIntermediate, false);
		}
		virtual ~InputModeManager() = default;
	public:

		// Class specific functions
		virtual Ref<Input::InputMode> InstantiateAssetIntoMemory(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
	};
}
