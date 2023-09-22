#include "kgpch.h"
#include "AudioContext.h"
#include "AudioMisc.h"

void AudioSource::init()
{
	alec(alGenSources(1, &sourceID));
	alec(alSource3f(sourceID, AL_POSITION, Position.x, Position.y, Position.z));
	alec(alSource3f(sourceID, AL_VELOCITY, Velocity.x, Velocity.y, Velocity.z));
	alec(alSourcef(sourceID, AL_PITCH, Pitch));
	alec(alSourcef(sourceID, AL_GAIN, Gain));
	alec(alSourcei(sourceID, AL_LOOPING, static_cast<ALint>(isLooping) ));
	alec(alSourcei(sourceID, AL_BUFFER, currentBuffer->bufferID));
}

void AudioSource::play() 
{
	alec(alSourcePlay(sourceID));
}
