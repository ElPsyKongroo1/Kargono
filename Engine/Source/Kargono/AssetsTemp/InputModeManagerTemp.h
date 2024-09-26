#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class InputModeManager : public AssetManagerTemp<Input::InputMode>
	{
	public:
		InputModeManager() : AssetManagerTemp<Input::InputMode>()
		{
			m_AssetName = "Input Mode";
			m_FileExtension = ".kginput";
			m_Flags.set(AssetManagerOptions::HasAssetCache, false);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, false);
			m_Flags.set(AssetManagerOptions::HasFileLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileImporting, false);
		}
		virtual ~InputModeManager() = default;
	public:

		// Class specific functions
		virtual Ref<Input::InputMode> InstantiateAssetIntoMemory(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
	};
}
