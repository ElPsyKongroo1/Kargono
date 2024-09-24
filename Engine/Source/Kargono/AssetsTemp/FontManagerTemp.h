#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class FontManager : public AssetManagerTemp<RuntimeUI::Font>
	{
	public:
		FontManager() = default;
		virtual ~FontManager() = default;
	public:
		// Override virtual functions
		virtual Ref<RuntimeUI::Font> InstantiateAssetIntoMemory(Assets::Asset& asset) override;
	};
}
