#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class ShaderManager : public AssetManagerTemp<Rendering::Shader>
	{
	public:
		ShaderManager() : AssetManagerTemp<Rendering::Shader>()
		{
			m_Flags.set(AssetManagerOptions::UseAssetCache, true);
		}
		virtual ~ShaderManager() = default;
	public:
		// Class specific functions
		virtual Ref<Rendering::Shader> InstantiateAssetIntoMemory(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
	};
}
