#include "kgpch.h"

#include "Kargono/Audio/AudioEngine.h"
#include "Kargono/Core/Buffer.h"
#include "Kargono/Assets/AssetManager.h"

#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <dr_wav.h>

namespace Kargono::Utility
{
#pragma once
#define OpenAL_ErrorCheck(message)\
{\
	ALenum error = alGetError();\
	if( error != AL_NO_ERROR)\
	{\
		KG_CORE_ERROR("OpenAL Error: {} with call for {}", error, #message);\
	}\
}

#define alec(FUNCTION_CALL)\
FUNCTION_CALL;\
OpenAL_ErrorCheck(FUNCTION_CALL)
}

namespace Kargono::Audio
{
	AudioEngine* AudioEngine::s_AudioContext = new AudioEngine();
	static AudioSourceSpecification s_DefaultSourceSpec =
	{
		{},
			{},
			1.0f,
			1.0f,
			false,
			nullptr
	};

	AudioSource::AudioSource()
	{
		alec(alGenSources(1, &m_SourceID));
	}

	AudioSource::~AudioSource()
	{
		alec(alSourceStop(m_SourceID));
		alec(alDeleteSources(1, &(m_SourceID)));
	}


	//void AudioSource::SetGain(float gain)
	//{
	//	KG_CORE_ASSERT(gain >= 0.0f && gain <= 100.0f, "Gain is outside of correct range. Range should be 0.0 - 100.0");
	//	m_Gain = (gain / 100) * 2.0f; // Max Range of Volume 
	//	alec(alSourcef(m_SourceID, AL_GAIN, m_Gain));
	//}


	AudioBuffer::AudioBuffer()
	{
		alec(alGenBuffers(1, &m_BufferID));
	}

	AudioBuffer::~AudioBuffer()
	{
		alec(alDeleteBuffers(1, &(m_BufferID)));
	}

	void AudioEngine::PlayStereoSound(Ref<AudioBuffer> audioBuffer)
	{
		static ALfloat forwardAndUpVectors[] =
		{
			0, 0, 1,  // Forward Vectors
			0, 1, 0   // Up Vectors
		};

		auto audioSource = s_AudioContext->m_StereoMusicSource.get();
		uint32_t sourceID = audioSource->GetSourceID();

		alec(alSourceStop(sourceID));
		if (!audioBuffer) { return; }
		alec(alSourcei(sourceID, AL_BUFFER, audioBuffer->m_BufferID));
		alec(alListener3f(AL_POSITION, 0, 0, 0));
		alec(alListener3f(AL_VELOCITY, 0, 0, 0));
		alec(alListenerfv(AL_ORIENTATION, forwardAndUpVectors));
		alec(alSourcePlay(sourceID));
	}
	void AudioEngine::PlaySound(const AudioSourceSpecification& sourceSpec, const AudioListenerSpecification& listenerSpec)
	{
		auto audioSource = s_AudioContext->m_AudioSourceQueue.front();
		uint32_t sourceID = audioSource->GetSourceID();

		alec(alSourceStop(sourceID));
		s_AudioContext->m_AudioSourceQueue.pop();
		alec(alSource3f(sourceID, AL_POSITION, sourceSpec.Position.x, sourceSpec.Position.y, sourceSpec.Position.z));
		alec(alSource3f(sourceID, AL_VELOCITY, sourceSpec.Velocity.x, sourceSpec.Velocity.y, sourceSpec.Velocity.z));
		alec(alSourcef(sourceID, AL_PITCH, sourceSpec.Pitch));
		alec(alSourcef(sourceID, AL_GAIN, sourceSpec.Gain));
		alec(alSourcei(sourceID, AL_LOOPING, static_cast<ALint>(sourceSpec.IsLooping)));
		if (!sourceSpec.CurrentBuffer) { return; }
		alec(alSourcei(sourceID, AL_BUFFER, sourceSpec.CurrentBuffer->m_BufferID));

		alec(alListener3f(AL_POSITION, listenerSpec.Position.x, listenerSpec.Position.y, listenerSpec.Position.z));
		alec(alListener3f(AL_VELOCITY, listenerSpec.Velocity.x, listenerSpec.Velocity.y, listenerSpec.Velocity.z));
		ALfloat forwardAndUpVectors[] =
		{
			listenerSpec.Forward.x, listenerSpec.Forward.y, listenerSpec.Forward.z,  // Forward Vectors
			listenerSpec.Up.x, listenerSpec.Up.y, listenerSpec.Up.z   // Up Vectors
		};
		alec(alListenerfv(AL_ORIENTATION, forwardAndUpVectors));

		alec(alSourcePlay(sourceID));

		s_AudioContext->m_AudioSourceQueue.push(audioSource);
		
	}

	void AudioEngine::PlaySound(Ref<AudioBuffer> audioBuffer)
	{
		s_DefaultSourceSpec.CurrentBuffer = audioBuffer;
		PlaySound(s_DefaultSourceSpec);
	}
	void AudioEngine::StopAllAudio()
	{
		for (uint32_t iterator{0}; iterator < s_AudioContext->m_AudioSourceQueue.size(); iterator++)
		{
			auto audioSource = s_AudioContext->m_AudioSourceQueue.front();
			alec(alSourceStop(audioSource->GetSourceID()));
			s_AudioContext->m_AudioSourceQueue.pop();
			s_AudioContext->m_AudioSourceQueue.push(audioSource);
		}

		alec(alSourceStop(s_AudioContext->m_StereoMusicSource->GetSourceID()));
	}
	void AudioEngine::Init()
	{
		// Find default audio device
		s_AudioContext->m_CurrentDeviceName = alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER);
		s_AudioContext->m_CurrentDeviceID = alcOpenDevice(s_AudioContext->m_CurrentDeviceName.c_str());
		KG_CORE_ASSERT(s_AudioContext->m_CurrentDeviceID, "Failed to get the default device for OpenAL");
		KG_CORE_INFO("OpenAL Device: {}", alcGetString(s_AudioContext->m_CurrentDeviceID, ALC_DEVICE_SPECIFIER));

		// Create an OpenAL audio context from the device
		s_AudioContext->m_ContextID = alcCreateContext(s_AudioContext->m_CurrentDeviceID, nullptr);
		//OpenAL_ErrorCheck(context);

		// Activate this context so that OpenAL state modifications are applied to the context
		bool makeCurrentValid = alcMakeContextCurrent(s_AudioContext->m_ContextID);
		KG_CORE_ASSERT(makeCurrentValid, "Failed to make the OpenAL context the current context");

		// Create a listener in 3D space
		s_AudioContext->m_DefaultListener = CreateScope<AudioListener>();

		// Initialize all Sound Sources
		s_AudioContext->m_StereoMusicSource = CreateScope<AudioSource>();
		alec(alSource3f(s_AudioContext->m_StereoMusicSource->GetSourceID(), AL_POSITION, 0, 0, 0));
		alec(alSource3f(s_AudioContext->m_StereoMusicSource->GetSourceID(), AL_VELOCITY, 0, 0, 0));
		alec(alSourcef(s_AudioContext->m_StereoMusicSource->GetSourceID(), AL_PITCH, 1.0f));
		alec(alSourcef(s_AudioContext->m_StereoMusicSource->GetSourceID(), AL_GAIN, 1.0f));
		alec(alSourcei(s_AudioContext->m_StereoMusicSource->GetSourceID(), AL_LOOPING, true));

		for (uint32_t iterator{0}; iterator < 15; iterator++)
		{
			s_AudioContext->m_AudioSourceQueue.push(CreateRef<AudioSource>());
		}
		KG_CORE_INFO("Audio Engine Initiated Successfully!");
	}

	void AudioEngine::Terminate()
	{
		s_DefaultSourceSpec.CurrentBuffer.reset();
		s_AudioContext->m_StereoMusicSource.reset();
		while (!s_AudioContext->m_AudioSourceQueue.empty())
		{
			KG_CORE_ASSERT(s_AudioContext->m_AudioSourceQueue.front().use_count() == 1, "Not all Audio Resources have been cleared!");
			s_AudioContext->m_AudioSourceQueue.front().reset();
			s_AudioContext->m_AudioSourceQueue.pop();
		}
		Assets::AssetManager::ClearAudioRegistry();

		// Close OpenAL Context
		alcDestroyContext(s_AudioContext->m_ContextID);
		alcMakeContextCurrent(nullptr);
		alcCloseDevice(s_AudioContext->m_CurrentDeviceID);
		s_AudioContext->m_CurrentDeviceName = "";
		s_AudioContext->m_CurrentDeviceID = nullptr;
		s_AudioContext->m_ContextID = nullptr;
		KG_CORE_INFO("Audio Engine Shutdown Successfully!");
	}
}


