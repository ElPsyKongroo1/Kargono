#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class ShaderManager : public AssetManagerTemp<Rendering::Shader>
	{
	public:
		ShaderManager() : AssetManagerTemp<Rendering::Shader>()
		{
			m_AssetName = "Shader";
			m_Flags.set(AssetManagerOptions::HasAssetCache, true);
			m_Flags.set(AssetManagerOptions::HasIntermediateLocation, true);
			m_Flags.set(AssetManagerOptions::HasFileLocation, false);
			m_Flags.set(AssetManagerOptions::HasFileImporting, false);
		}
		virtual ~ShaderManager() = default;
	public:
		// Class specific functions
		virtual Ref<Rendering::Shader> InstantiateAssetIntoMemory(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
	};
}
