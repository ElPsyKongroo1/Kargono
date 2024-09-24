#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class InputModeManager : public AssetManagerTemp<Input::InputMode>
	{
	public:
		InputModeManager() : AssetManagerTemp<Input::InputMode>()
		{
			m_Flags.set(AssetManagerOptions::UseRuntimeCache, false);
		}
		virtual ~InputModeManager() = default;
	public:
		// Override virtual functions
		virtual Ref<Input::InputMode> InstantiateAssetIntoMemory(Assets::Asset& asset) override;

		// Class specific functions
		bool DeserializeInputMode(Ref<Input::InputMode> inputMode, const std::filesystem::path& filepath);
	};
}
