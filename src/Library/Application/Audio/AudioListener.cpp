#include "AudioContext.h"
#include "AudioMisc.h"

void AudioListener::init()
{
	alec(alListener3f(AL_POSITION, Position.x, Position.y, Position.z));
	alec(alListener3f(AL_VELOCITY, Velocity.x, Velocity.y, Velocity.z));
	ALfloat forwardAndUpVectors[] =
	{
		Forward.x, Forward.y, Forward.z,  // Forward Vectors
		Up.x, Up.y, Up.z   // Up Vectors
	};
	alec(alListenerfv(AL_ORIENTATION, forwardAndUpVectors));
}