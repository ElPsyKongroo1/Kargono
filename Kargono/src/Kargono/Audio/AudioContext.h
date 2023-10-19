#pragma once

#include <dr_wav.h>
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <glm/glm.hpp>

class AudioListener 
{
public:
	glm::vec3 Position;
	glm::vec3 Velocity;
	glm::vec3 Forward;
	glm::vec3 Up;
private:
	void init();
public:
	AudioListener() : Position {}, Velocity {}, Forward {}, Up {} {}
	AudioListener(glm::vec3 position, glm::vec3 velocity,
			      glm::vec3 forward, glm::vec3 up) : Position{position}, Velocity{velocity},
													 Forward{forward}, Up{up} 
	{
		init();
	}
};
class AudioBuffer
{
public:
	ALuint bufferID;
	unsigned int channels = 0;
	unsigned int sampleRate = 0;
	drwav_uint64 totalPCMFrameCount = 0;
	std::vector<uint16_t> pcmData;
public:
	AudioBuffer(const char* fileLocation) : bufferID{0}, channels { 0 }, sampleRate{ 0 },
		totalPCMFrameCount{ 0 }, pcmData{ std::vector<uint16_t>() } 
	{
		init(fileLocation);
	}
	AudioBuffer() : bufferID{ 0 }, channels{ 0 }, sampleRate{ 0 },
		totalPCMFrameCount{ 0 }, pcmData{ std::vector<uint16_t>() }
	{
	
	}
private:
	void init(const char* fileLocation);
private:
	drwav_uint64 getTotalSamples() { return totalPCMFrameCount * channels; }
};

class AudioSource
{
public:
	ALuint sourceID;
	glm::vec3 Position;
	glm::vec3 Velocity;
	ALfloat Pitch;
	ALfloat m_Gain;
	ALfloat isLooping;
	AudioBuffer* currentBuffer;
public: 
	void init();
	void play();

	void SetGain(float gain);
public:
	AudioSource() : sourceID{ 0 }, Position{}, Velocity{},
		Pitch{}, m_Gain{}, isLooping{}, currentBuffer{nullptr} 
	{
	
	}
	AudioSource(glm::vec3 Position, glm::vec3 Velocity,
		ALfloat Pitch, ALfloat Gain, ALfloat isLooping, AudioBuffer* currentBuffer ) : Position{Position},
		Velocity{ Velocity }, Pitch{ Pitch }, m_Gain{ Gain }, isLooping{isLooping}, currentBuffer{currentBuffer}
	{
		init();
	}
};

class AudioContext 
{
private:
	void init(const char* initStereoAudio);
public:
	void terminate();
public:
	const ALCchar* currentDeviceString;
	ALCdevice* device;
	ALCcontext* context;
	AudioListener* defaultListener;
	AudioSource* m_DefaultStereoSource;
	AudioBuffer* defaultBuffer;
	std::vector<AudioBuffer*> allAudioBuffers;
	std::vector<AudioSource*> allAudioSources;
public:
	AudioContext(const char* initStereoAudio = nullptr) : currentDeviceString{ nullptr }, device{ nullptr }, context{ nullptr },
		allAudioBuffers{ std::vector<AudioBuffer*>() }, allAudioSources{std::vector<AudioSource*>()}
	{
		init(initStereoAudio);
	}

};
