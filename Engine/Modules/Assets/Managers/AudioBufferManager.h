#pragma once

#include "Modules/Assets/AssetManager.h"
#include "Modules/Audio/Assets/AudioBuffer.h"

namespace Kargono::Assets
{
	extern AssetManager<Audio::AudioBuffer> s_AudioBufferManager;
}