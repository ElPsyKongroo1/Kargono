#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Math/Math.h"
#include "Modules/Assets/AssetsCommon.h"
#include "Modules/Audio/AudioSource.h"
#include "Modules/Audio/AudioListener.h"

#include <queue>
#include <string>

struct ALCdevice;
struct ALCcontext;

namespace Kargono::Audio
{
	class AudioService;
	class AudioBuffer;

	class AudioContext
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		AudioContext() = default;
		~AudioContext() = default;

		//==============================
		// LifeCycle Functions
		//==============================
		bool Init();
		bool Terminate();

		//==============================
		// Sound Playback
		//==============================
		void PlayStereoSound(Assets::AssetRef<AudioBuffer> audioBuffer);
		void PlayStereoSoundFromHandle(Assets::AssetHandle audioHandle);
		void PlaySound(AudioSourceSpecification& sourceSpec, AudioListenerSpecification& listenerSpec);
		void PlaySound(Assets::AssetRef<AudioBuffer> audioBuffer);
		void PlaySoundFromHandle(Assets::AssetHandle audioHandle);
		void StopAllAudio();
		//==============================
		// Manage Sound State
		//==============================
		void SetMute(bool isMute);
		
	private:
		//==============================
		// Internal Fields
		//==============================
		// Debug information
		std::string m_CurrentDeviceName{};
		// Backend handles
		ALCdevice* m_CurrentDeviceID{ nullptr };
		ALCcontext* m_ContextID{ nullptr };
		// Listeners and sources
		Scope<AudioListener> m_DefaultListener{ nullptr };
		AudioSourceSpecification m_DefaultSourceSpec{};
		std::queue<Ref<AudioSource>> m_AudioSourceQueue{};
		Scope<AudioSource> m_StereoMusicSource{ nullptr };
		// Local state
		bool m_Mute{ false };
	};

	class AudioService // TODO: REMOVE EWWWWWWW
	{
	public:
		//==============================
		// Create Audio Context
		//==============================
		static void CreateAudioContext()
		{
			// Initialize AudioContext
			if (!s_AudioContext)
			{
				s_AudioContext = CreateRef<Audio::AudioContext>();
			}

			// Verify init is successful
			KG_VERIFY(s_AudioContext, "Audio Service System Initiated");
		}
		static void RemoveAudioContext()
		{
			// Clear AudioContext
			s_AudioContext.reset();
			s_AudioContext = nullptr;

			// Verify terminate is successful
			KG_VERIFY(!s_AudioContext, "Audio Service System Initiated");
		}
		//==============================
		// Getters/Setters
		//==============================
		static AudioContext& GetActiveContext() { return *s_AudioContext; }
		static bool IsContextActive() { return (bool)s_AudioContext; }
	private:
		//==============================
		// Internal Fields
		//==============================
		static inline Ref<AudioContext> s_AudioContext{ nullptr };
	};
}
