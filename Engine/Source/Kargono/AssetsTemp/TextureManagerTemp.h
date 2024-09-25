#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class TextureManager : public AssetManagerTemp<Rendering::Texture2D>
	{
	public:
		TextureManager() : AssetManagerTemp<Rendering::Texture2D>()
		{
			m_Flags.set(AssetManagerOptions::UseAssetCache, true);
		}
		virtual ~TextureManager() = default;
	public:
		// Class specific functions
		virtual Ref<Rendering::Texture2D> InstantiateAssetIntoMemory(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
	};
}
