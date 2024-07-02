#include "kgpch.h"

#include "Kargono/Audio/Audio.h"
#include "Kargono/Core/Buffer.h"
#include "Kargono/Assets/AssetManager.h"

#include "API/Audio/OpenALAPI.h"

namespace Kargono::Audio
{
	AudioContext* AudioService::s_AudioContext = new AudioContext();
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
		CallAndCheckALError(alGenSources(1, &m_SourceID));
	}

	AudioSource::~AudioSource()
	{
		CallAndCheckALError(alSourceStop(m_SourceID));
		CallAndCheckALError(alDeleteSources(1, &(m_SourceID)));
	}


	//void AudioSource::SetGain(float gain)
	//{
	//	KG_CORE_ASSERT(gain >= 0.0f && gain <= 100.0f, "Gain is outside of correct range. Range should be 0.0 - 100.0");
	//	m_Gain = (gain / 100) * 2.0f; // Max Range of Volume 
	//	alec(alSourcef(m_SourceID, AL_GAIN, m_Gain));
	//}


	AudioBuffer::AudioBuffer()
	{
		CallAndCheckALError(alGenBuffers(1, &m_BufferID));
	}

	AudioBuffer::~AudioBuffer()
	{
		CallAndCheckALError(alDeleteBuffers(1, &(m_BufferID)));
	}

	void AudioService::PlayStereoSound(Ref<AudioBuffer> audioBuffer)
	{
		static ALfloat forwardAndUpVectors[] =
		{
			0, 0, 1,  // Forward Vectors
			0, 1, 0   // Up Vectors
		};

		auto audioSource = s_AudioContext->StereoMusicSource.get();
		uint32_t sourceID = audioSource->GetSourceID();

		CallAndCheckALError(alSourceStop(sourceID));
		if (!audioBuffer) { return; }
		CallAndCheckALError(alSourcei(sourceID, AL_BUFFER, audioBuffer->m_BufferID));
		CallAndCheckALError(alListener3f(AL_POSITION, 0, 0, 0));
		CallAndCheckALError(alListener3f(AL_VELOCITY, 0, 0, 0));
		CallAndCheckALError(alListenerfv(AL_ORIENTATION, forwardAndUpVectors));
		CallAndCheckALError(alSourcePlay(sourceID));
	}
	void AudioService::PlayStereoSoundFromName(const std::string& audioName)
	{
		auto [handle, audioBuffer] = Assets::AssetManager::GetAudio(audioName);
		if (audioBuffer)
		{
			PlayStereoSound(audioBuffer);
		}
	}
	void AudioService::PlaySound(const AudioSourceSpecification& sourceSpec, const AudioListenerSpecification& listenerSpec)
	{
		auto audioSource = s_AudioContext->AudioSourceQueue.front();
		uint32_t sourceID = audioSource->GetSourceID();

		CallAndCheckALError(alSourceStop(sourceID));
		s_AudioContext->AudioSourceQueue.pop();
		CallAndCheckALError(alSource3f(sourceID, AL_POSITION, sourceSpec.Position.x, sourceSpec.Position.y, sourceSpec.Position.z));
		CallAndCheckALError(alSource3f(sourceID, AL_VELOCITY, sourceSpec.Velocity.x, sourceSpec.Velocity.y, sourceSpec.Velocity.z));
		CallAndCheckALError(alSourcef(sourceID, AL_PITCH, sourceSpec.Pitch));
		CallAndCheckALError(alSourcef(sourceID, AL_GAIN, sourceSpec.Gain));
		CallAndCheckALError(alSourcei(sourceID, AL_LOOPING, static_cast<ALint>(sourceSpec.IsLooping)));
		if (!sourceSpec.CurrentBuffer) { return; }
		CallAndCheckALError(alSourcei(sourceID, AL_BUFFER, sourceSpec.CurrentBuffer->m_BufferID));

		CallAndCheckALError(alListener3f(AL_POSITION, listenerSpec.Position.x, listenerSpec.Position.y, listenerSpec.Position.z));
		CallAndCheckALError(alListener3f(AL_VELOCITY, listenerSpec.Velocity.x, listenerSpec.Velocity.y, listenerSpec.Velocity.z));
		ALfloat forwardAndUpVectors[] =
		{
			listenerSpec.Forward.x, listenerSpec.Forward.y, listenerSpec.Forward.z,  // Forward Vectors
			listenerSpec.Up.x, listenerSpec.Up.y, listenerSpec.Up.z   // Up Vectors
		};
		CallAndCheckALError(alListenerfv(AL_ORIENTATION, forwardAndUpVectors));

		CallAndCheckALError(alSourcePlay(sourceID));

		s_AudioContext->AudioSourceQueue.push(audioSource);
		
	}

	void AudioService::PlaySound(Ref<AudioBuffer> audioBuffer)
	{
		s_DefaultSourceSpec.CurrentBuffer = audioBuffer;
		PlaySound(s_DefaultSourceSpec);
	}

	void AudioService::PlaySoundFromName(const std::string& audioName)
	{
		auto [handle, audioBuffer] = Assets::AssetManager::GetAudio(audioName);
		if (audioBuffer)
		{
			Audio::AudioService::PlaySound(audioBuffer);
		}
	}
	void AudioService::StopAllAudio()
	{
		for (uint32_t iterator{0}; iterator < s_AudioContext->AudioSourceQueue.size(); iterator++)
		{
			auto audioSource = s_AudioContext->AudioSourceQueue.front();
			CallAndCheckALError(alSourceStop(audioSource->GetSourceID()));
			s_AudioContext->AudioSourceQueue.pop();
			s_AudioContext->AudioSourceQueue.push(audioSource);
		}

		CallAndCheckALError(alSourceStop(s_AudioContext->StereoMusicSource->GetSourceID()));
	}
	void AudioService::Init()
	{
		// Find default audio device
		s_AudioContext->CurrentDeviceName = alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER);
		s_AudioContext->CurrentDeviceID = alcOpenDevice(s_AudioContext->CurrentDeviceName.c_str());
		KG_ASSERT(s_AudioContext->CurrentDeviceID, "Failed to get the default device for OpenAL");
		//KG_INFO("OpenAL Device: {}", alcGetString(s_AudioContext->m_CurrentDeviceID, ALC_DEVICE_SPECIFIER));

		// Create an OpenAL audio context from the device
		s_AudioContext->ContextID = alcCreateContext(s_AudioContext->CurrentDeviceID, nullptr);
		//OpenAL_ErrorCheck(context);

		// Activate this context so that OpenAL state modifications are applied to the context
		bool makeCurrentValid = alcMakeContextCurrent(s_AudioContext->ContextID);
		KG_ASSERT(makeCurrentValid, "Failed to make the OpenAL context the current context");

		// Create a listener in 3D space
		s_AudioContext->DefaultListener = CreateScope<AudioListener>();

		// Initialize all Sound Sources
		s_AudioContext->StereoMusicSource = CreateScope<AudioSource>();
		CallAndCheckALError(alSource3f(s_AudioContext->StereoMusicSource->GetSourceID(), AL_POSITION, 0, 0, 0));
		CallAndCheckALError(alSource3f(s_AudioContext->StereoMusicSource->GetSourceID(), AL_VELOCITY, 0, 0, 0));
		CallAndCheckALError(alSourcef(s_AudioContext->StereoMusicSource->GetSourceID(), AL_PITCH, 1.0f));
		CallAndCheckALError(alSourcef(s_AudioContext->StereoMusicSource->GetSourceID(), AL_GAIN, 1.0f));
		CallAndCheckALError(alSourcei(s_AudioContext->StereoMusicSource->GetSourceID(), AL_LOOPING, true));

		for (uint32_t iterator{0}; iterator < 15; iterator++)
		{
			s_AudioContext->AudioSourceQueue.push(CreateRef<AudioSource>());
		}
		KG_VERIFY(s_AudioContext, "Audio Engine Init");
	}

	void AudioService::Terminate()
	{
		s_DefaultSourceSpec.CurrentBuffer.reset();
		s_AudioContext->StereoMusicSource.reset();
		while (!s_AudioContext->AudioSourceQueue.empty())
		{
			KG_ASSERT(s_AudioContext->AudioSourceQueue.front().use_count() == 1, "Not all Audio Resources have been cleared!");
			s_AudioContext->AudioSourceQueue.front().reset();
			s_AudioContext->AudioSourceQueue.pop();
		}
		Assets::AssetManager::ClearAudioRegistry();

		// Close OpenAL Context
		alcDestroyContext(s_AudioContext->ContextID);
		alcMakeContextCurrent(nullptr);
		alcCloseDevice(s_AudioContext->CurrentDeviceID);
		s_AudioContext->CurrentDeviceName = "";
		s_AudioContext->CurrentDeviceID = nullptr;
		s_AudioContext->ContextID = nullptr;
		s_AudioContext = nullptr;
		KG_VERIFY(!s_AudioContext, "Close Audio Engine");
	}
}


