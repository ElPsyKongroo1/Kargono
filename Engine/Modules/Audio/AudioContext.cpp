#include "kgpch.h"

#include "Modules/Audio/AudioContext.h"
#include "Kargono/Core/Buffer.h"
#include "Modules/Assets/AssetService.h"
#include "Modules/Audio/Assets/AudioBuffer.h"
#include "Modules/Audio/AudioSource.h"
#include "Modules/Audio/AudioListener.h"

#include "Modules/Audio/ExternalAPI/OpenALAPI.h"

namespace Kargono::Audio
{
	void AudioContext::PlayStereoSound(Assets::AssetRef<AudioBuffer> audioBuffer)
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

		AudioSource* audioSource = m_StereoMusicSource.get();
		SourceID sourceID = audioSource->GetSourceID();

		CallAndCheckALError(alSourceStop(sourceID));
		if (!audioBuffer) 
		{
			return;
		}
		CallAndCheckALError(alSourcei(sourceID, AL_BUFFER, audioBuffer->m_BufferID));
		CallAndCheckALError(alListener3f(AL_POSITION, 0, 0, 0));
		CallAndCheckALError(alListener3f(AL_VELOCITY, 0, 0, 0));
		CallAndCheckALError(alListenerfv(AL_ORIENTATION, forwardAndUpVectors));
		CallAndCheckALError(alSourcePlay(sourceID));
	}
	void AudioContext::PlayStereoSoundFromHandle(Assets::AssetHandle audioHandle)
	{
		Assets::AssetRef<AudioBuffer> audioBuffer = Assets::AssetService::m_AudioBufferManager.GetAssetByHandle(audioHandle);
		if (audioBuffer)
		{
			PlayStereoSound(audioBuffer);
		}
		else
		{
			KG_WARN("Could not find an audio buffer with the provided handle {}", audioHandle);
		}
	}
	void AudioContext::PlaySound(AudioSourceSpecification& sourceSpec, AudioListenerSpecification& listenerSpec)
	{
		if (m_Mute)
		{
			return;
		}
		Ref<AudioSource> audioSource = m_AudioSourceQueue.front();
		SourceID sourceID = audioSource->GetSourceID();

		CallAndCheckALError(alSourceStop(sourceID));
		m_AudioSourceQueue.pop();
		CallAndCheckALError(alSource3f(sourceID, AL_POSITION, sourceSpec.m_Position.x, sourceSpec.m_Position.y, sourceSpec.m_Position.z));
		CallAndCheckALError(alSource3f(sourceID, AL_VELOCITY, sourceSpec.m_Velocity.x, sourceSpec.m_Velocity.y, sourceSpec.m_Velocity.z));
		CallAndCheckALError(alSourcef(sourceID, AL_PITCH, sourceSpec.m_Pitch));
		CallAndCheckALError(alSourcef(sourceID, AL_GAIN, sourceSpec.m_Gain));
		CallAndCheckALError(alSourcei(sourceID, AL_LOOPING, static_cast<ALint>(sourceSpec.m_IsLooping)));
		if (!sourceSpec.m_CurrentBuffer) { return; }
		CallAndCheckALError(alSourcei(sourceID, AL_BUFFER, sourceSpec.m_CurrentBuffer->m_BufferID));

		CallAndCheckALError(alListener3f(AL_POSITION, listenerSpec.m_Position.x, listenerSpec.m_Position.y, listenerSpec.m_Position.z));
		CallAndCheckALError(alListener3f(AL_VELOCITY, listenerSpec.m_Velocity.x, listenerSpec.m_Velocity.y, listenerSpec.m_Velocity.z));
		ALfloat forwardAndUpVectors[] =
		{
			listenerSpec.m_Forward.x, listenerSpec.m_Forward.y, listenerSpec.m_Forward.z,  // Forward Vectors
			listenerSpec.m_Up.x, listenerSpec.m_Up.y, listenerSpec.m_Up.z   // Up Vectors
		};
		CallAndCheckALError(alListenerfv(AL_ORIENTATION, forwardAndUpVectors));

		CallAndCheckALError(alSourcePlay(sourceID));

		m_AudioSourceQueue.push(audioSource);
		
	}

	void AudioContext::PlaySound(Assets::AssetRef<AudioBuffer> audioBuffer)
	{
		m_DefaultSourceSpec.m_CurrentBuffer = audioBuffer;
		static AudioListenerSpecification defaultListenerSpec{};
		PlaySound(m_DefaultSourceSpec, defaultListenerSpec);
	}

	void AudioContext::PlaySoundFromHandle(Assets::AssetHandle audioHandle)
	{
		Assets::AssetRef<AudioBuffer> audioBuffer = Assets::AssetService::m_AudioBufferManager.GetAssetByHandle(audioHandle);
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
		for (size_t iterator{ 0 }; iterator < m_AudioSourceQueue.size(); iterator++)
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

		for (size_t iterator{ 0 }; iterator < 15; iterator++)
		{
			m_AudioSourceQueue.push(CreateRef<AudioSource>());
		}
		return true;
	}

	bool AudioContext::Terminate()
	{
		m_DefaultSourceSpec.m_CurrentBuffer.Reset();
		m_StereoMusicSource.reset();
		while (!m_AudioSourceQueue.empty())
		{
			KG_ASSERT(m_AudioSourceQueue.front().use_count() == 1, "Not all Audio Resources have been cleared!");
			m_AudioSourceQueue.front().reset();
			m_AudioSourceQueue.pop();
		}
		Assets::AssetService::m_AudioBufferManager.ClearAssetRegistry();

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


