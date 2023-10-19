#include "kgpch.h"

#include "Kargono/Audio/AudioContext.h"
#include "Kargono/Audio/AudioMisc.h"

void AudioContext::init(const char* initStereoAudio) 
{
	// Find default audio device
	currentDeviceString = alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER);
	device = alcOpenDevice(currentDeviceString);
	if (!device)
	{
		std::cerr << "Failed to get the default device for OpenAL\n";
		throw std::runtime_error("Check Logs");
	}
	std::cout << "OpenAL Device: " << alcGetString(device, ALC_DEVICE_SPECIFIER) << '\n';
	//OpenAL_ErrorCheck(device);

	// Create an OpenAL audio context from the device
	context = alcCreateContext(device, nullptr);
	//OpenAL_ErrorCheck(context);

	// Activate this context so that OpenAL state modifications are applied to the context
	if (!alcMakeContextCurrent(context))
	{
		std::cerr << "Failed to make the OpenAL context the current context\n";
		throw std::runtime_error("Check Logs");
	}
	OpenAL_ErrorCheck("Make context current");

	// Create a listener in 3D space
	defaultListener = new AudioListener(glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 0.0f));

	if (initStereoAudio)
	{
		defaultBuffer = new AudioBuffer(initStereoAudio);
		allAudioBuffers.push_back(defaultBuffer);
		m_DefaultStereoSource = new AudioSource(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
			1.0f, 0.2f, AL_TRUE, defaultBuffer);
	}
	else
	{
		m_DefaultStereoSource = new AudioSource(glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
			1.0f, 0.2f, AL_TRUE, nullptr);
	}
	allAudioSources.push_back(m_DefaultStereoSource);
	
}

void AudioContext::terminate()
{
	std::for_each(allAudioSources.begin(), allAudioSources.end(), [](AudioSource* source)
		{
			if (source)
			{
				alec(alDeleteSources(1, &(source->sourceID)));
				delete source;
			}
			source = nullptr;
		});
	allAudioSources.clear();

	std::for_each(allAudioBuffers.begin(), allAudioBuffers.end(), [](AudioBuffer* buffer) 
		{
			if (buffer) 
			{
				alec(alDeleteBuffers(1, &(buffer->bufferID)));
				delete buffer;
			}
			buffer = nullptr;
		});
	allAudioBuffers.clear();

	alcDestroyContext(context);
	alcMakeContextCurrent(nullptr);
	alcCloseDevice(device);
	currentDeviceString = "";
	device = nullptr;
	context = nullptr;
	delete defaultListener;
	defaultListener = nullptr;
	defaultBuffer = nullptr;
	m_DefaultStereoSource = nullptr;
}




