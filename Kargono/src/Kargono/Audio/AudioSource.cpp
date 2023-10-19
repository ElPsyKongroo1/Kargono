#include "kgpch.h"

#include "Kargono/Audio/AudioContext.h"
#include "Kargono/Audio/AudioMisc.h"

void AudioSource::init()
{
	alec(alGenSources(1, &sourceID));
	alec(alSource3f(sourceID, AL_POSITION, Position.x, Position.y, Position.z));
	alec(alSource3f(sourceID, AL_VELOCITY, Velocity.x, Velocity.y, Velocity.z));
	alec(alSourcef(sourceID, AL_PITCH, Pitch));
	alec(alSourcef(sourceID, AL_GAIN, m_Gain));
	alec(alSourcei(sourceID, AL_LOOPING, static_cast<ALint>(isLooping) ));
	if (!currentBuffer) { return; }
	alec(alSourcei(sourceID, AL_BUFFER, currentBuffer->bufferID));
}

void AudioSource::play() 
{
	if (!currentBuffer) { KG_CORE_ERROR("No Audio Buffer Loaded into Source!"); return; }

	alec(alSourcePlay(sourceID));
}

void AudioSource::SetGain(float gain)
{
	KG_CORE_ASSERT(gain >= 0.0f && gain <= 100.0f, "Gain is outside of correct range. Range should be 0.0 - 100.0");
	m_Gain = (gain / 100) * 2.0f; // Max Range of Volume 
	alec(alSourcef(sourceID, AL_GAIN, m_Gain));
}
