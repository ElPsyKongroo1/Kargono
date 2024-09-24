#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class TextureManager : public AssetManagerTemp<Rendering::Texture2D>
	{
	public:
		TextureManager() = default;
		virtual ~TextureManager() = default;
	public:
		// Override virtual functions
		virtual Ref<Rendering::Texture2D> InstantiateAssetIntoMemory(Assets::Asset& asset) override;
	};
}
