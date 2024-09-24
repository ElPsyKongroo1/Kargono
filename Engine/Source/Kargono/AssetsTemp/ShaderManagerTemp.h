#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class ShaderManager : public AssetManagerTemp<Rendering::Shader>
	{
	public:
		ShaderManager() = default;
		virtual ~ShaderManager() = default;
	public:
		// Override virtual functions
		virtual Ref<Rendering::Shader> InstantiateAssetIntoMemory(Assets::Asset& asset) override;
	};
}
