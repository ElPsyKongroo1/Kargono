#pragma once
#include "Kargono/AssetsTemp/AssetManagerTemp.h"

namespace Kargono::Assets
{
	class AudioManager : public AssetManagerTemp<Kargono::Audio::AudioBuffer>
	{
	public:
		AudioManager() = default;
		virtual ~AudioManager() = default;
	public:
		// Override virtual functions
		virtual Ref<Kargono::Audio::AudioBuffer> InstantiateAssetIntoMemory(Assets::Asset& asset) override;
	};
}
