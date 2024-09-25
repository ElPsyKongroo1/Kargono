#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class FontManager : public AssetManagerTemp<RuntimeUI::Font>
	{
	public:
		FontManager() : AssetManagerTemp<RuntimeUI::Font>()
		{
			m_Flags.set(AssetManagerOptions::UseAssetCache, true);
			m_Flags.set(AssetManagerOptions::CreateAssetIntermediate, true);
		}
		virtual ~FontManager() = default;
	public:

		// Class specific functions
		virtual Ref<RuntimeUI::Font> InstantiateAssetIntoMemory(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
	};
}
