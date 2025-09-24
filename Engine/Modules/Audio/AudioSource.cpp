#include "kgpch.h"

#include "Modules/Audio/AudioSource.h"
#include "Modules/Audio/ExternalAPI/OpenALAPI.h"

namespace Kargono::Audio
{
	AudioSource::AudioSource()
	{
		CallAndCheckALError(alGenSources(1, &m_SourceID));
	}

	AudioSource::~AudioSource()
	{
		CallAndCheckALError(alSourceStop(m_SourceID));
		CallAndCheckALError(alDeleteSources(1, &(m_SourceID)));
	}
}