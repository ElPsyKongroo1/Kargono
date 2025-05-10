#include "kgpch.h"

#include "Modules/Audio/Audio.h"
#include "Kargono/Core/Buffer.h"
#include "Modules/Assets/AssetService.h"

#include "Modules/Audio/ExternalAPI/OpenALAPI.h"

namespace Kargono::Audio
{
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


	AudioBuffer::AudioBuffer()
	{
		CallAndCheckALError(alGenBuffers(1, &m_BufferID));
	}

	AudioBuffer::~AudioBuffer()
	{
		CallAndCheckALError(alDeleteBuffers(1, &(m_BufferID)));
	}

	void AudioContext::PlayStereoSound(Ref<AudioBuffer> audioBuffer)
	{
		if (m_Mute)
		{
			return;
		}
		static ALfloat forwardAndUpVectors[] =
		{
			0, 0, 1,  // Forward Vectors
			0, 1, 0   // Up Vectors
		};

		auto audioSource = m_StereoMusicSource.get();
		uint32_t sourceID = audioSource->GetSourceID();

		CallAndCheckALError(alSourceStop(sourceID));
		if (!audioBuffer) { return; }
		CallAndCheckALError(alSourcei(sourceID, AL_BUFFER, audioBuffer->m_BufferID));
		CallAndCheckALError(alListener3f(AL_POSITION, 0, 0, 0));
		CallAndCheckALError(alListener3f(AL_VELOCITY, 0, 0, 0));
		CallAndCheckALError(alListenerfv(AL_ORIENTATION, forwardAndUpVectors));
		CallAndCheckALError(alSourcePlay(sourceID));
	}
	void AudioContext::PlayStereoSoundFromHandle(Assets::AssetHandle audioHandle)
	{
		Ref<Audio::AudioBuffer> audioBuffer = Assets::AssetService::GetAudioBuffer(audioHandle);
		if (audioBuffer)
		{
			PlayStereoSound(audioBuffer);
		}
		else
		{
			KG_WARN("Could not find an audio buffer with the provided handle {}", audioHandle);
		}
	}
	void AudioContext::PlaySound(const AudioSourceSpecification& sourceSpec, const AudioListenerSpecification& listenerSpec)
	{
		if (m_Mute)
		{
			return;
		}
		auto audioSource = m_AudioSourceQueue.front();
		uint32_t sourceID = audioSource->GetSourceID();

		CallAndCheckALError(alSourceStop(sourceID));
		m_AudioSourceQueue.pop();
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

		m_AudioSourceQueue.push(audioSource);
		
	}

	void AudioContext::PlaySound(Ref<AudioBuffer> audioBuffer)
	{
		s_DefaultSourceSpec.CurrentBuffer = audioBuffer;
		PlaySound(s_DefaultSourceSpec);
	}

	void AudioContext::PlaySoundFromHandle(Assets::AssetHandle audioHandle)
	{
		Ref<Audio::AudioBuffer> audioBuffer = Assets::AssetService::GetAudioBuffer(audioHandle);
		if (audioBuffer)
		{
			Audio::AudioContext::PlaySound(audioBuffer);
		}
		else
		{
			KG_WARN("Could not find an audio buffer with the provided handle {}", audioHandle);
		}
	}
	void AudioContext::SetMute(bool isMute)
	{
		if (isMute)
		{
			StopAllAudio();
			m_Mute = true;
		}
		else
		{
			m_Mute = false;
		}
	}
	void AudioContext::StopAllAudio()
	{
		for (uint32_t iterator{0}; iterator < m_AudioSourceQueue.size(); iterator++)
		{
			Ref<AudioSource> audioSource = m_AudioSourceQueue.front();
			CallAndCheckALError(alSourceStop(audioSource->GetSourceID()));
			m_AudioSourceQueue.pop();
			m_AudioSourceQueue.push(audioSource);
		}

		CallAndCheckALError(alSourceStop(m_StereoMusicSource->GetSourceID()));
	}
	bool AudioContext::Init()
	{
		// Find default audio device
		m_CurrentDeviceName = alcGetString(nullptr, ALC_DEFAULT_DEVICE_SPECIFIER);
		m_CurrentDeviceID = alcOpenDevice(m_CurrentDeviceName.c_str());
		if (!m_CurrentDeviceID)
		{
			KG_ERROR("Failed to get the default device for OpenAL");
			return false;
		}
		//KG_INFO("OpenAL Device: {}", alcGetString(m_CurrentDeviceID, ALC_DEVICE_SPECIFIER));

		// Create an OpenAL audio context from the device
		m_ContextID = alcCreateContext(m_CurrentDeviceID, nullptr);
		//OpenAL_ErrorCheck(context);

		// Activate this context so that OpenAL state modifications are applied to the context
		if (!alcMakeContextCurrent(m_ContextID))
		{
			KG_ERROR("Failed to make the OpenAL context the current context");
			return false;
		}

		// Create a listener in 3D space
		m_DefaultListener = CreateScope<AudioListener>();

		// Initialize all Sound Sources
		m_StereoMusicSource = CreateScope<AudioSource>();
		CallAndCheckALError(alSource3f(m_StereoMusicSource->GetSourceID(), AL_POSITION, 0, 0, 0));
		CallAndCheckALError(alSource3f(m_StereoMusicSource->GetSourceID(), AL_VELOCITY, 0, 0, 0));
		CallAndCheckALError(alSourcef(m_StereoMusicSource->GetSourceID(), AL_PITCH, 1.0f));
		CallAndCheckALError(alSourcef(m_StereoMusicSource->GetSourceID(), AL_GAIN, 1.0f));
		CallAndCheckALError(alSourcei(m_StereoMusicSource->GetSourceID(), AL_LOOPING, true));

		for (uint32_t iterator{0}; iterator < 15; iterator++)
		{
			m_AudioSourceQueue.push(CreateRef<AudioSource>());
		}
		return true;
	}

	bool AudioContext::Terminate()
	{
		s_DefaultSourceSpec.CurrentBuffer.reset();
		m_StereoMusicSource.reset();
		while (!m_AudioSourceQueue.empty())
		{
			KG_ASSERT(m_AudioSourceQueue.front().use_count() == 1, "Not all Audio Resources have been cleared!");
			m_AudioSourceQueue.front().reset();
			m_AudioSourceQueue.pop();
		}
		Assets::AssetService::ClearAudioBufferRegistry();

		// Close OpenAL Context
		alcDestroyContext(m_ContextID);
		alcMakeContextCurrent(nullptr);
		alcCloseDevice(m_CurrentDeviceID);
		m_CurrentDeviceName = "";
		m_CurrentDeviceID = nullptr;
		m_ContextID = nullptr;
		return true;
	}
}


