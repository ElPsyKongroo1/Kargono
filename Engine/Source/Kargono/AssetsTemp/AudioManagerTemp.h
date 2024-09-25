#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class AudioManager : public AssetManagerTemp<Audio::AudioBuffer>
	{
	public:
		AudioManager() : AssetManagerTemp<Audio::AudioBuffer>()
		{
			m_Flags.set(AssetManagerOptions::UseAssetCache, true);
			m_Flags.set(AssetManagerOptions::CreateAssetIntermediate, true);
		}
		virtual ~AudioManager() = default;
	public:
		// Functions specific to this manager type
		virtual Ref<Audio::AudioBuffer> InstantiateAssetIntoMemory(Assets::Asset& asset, const std::filesystem::path& assetPath) override;
	};
}
