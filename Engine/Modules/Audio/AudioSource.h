#pragma once

#include "Kargono/Math/MathAliases.h"
#include "Kargono/Core/Base.h"
#include "Modules/Assets/AssetReference.h"
#include "Modules/Audio/Assets/AudioBuffer.h"

#include <cstdint>

namespace Kargono::Audio
{
	using SourceID = uint32_t;
	constexpr SourceID k_InvalidSourceID{ 0 }; // TODO: Maybe change to upperbound??

	struct AudioSourceSpecification
	{
	public:
		Math::vec3 m_Position{};
		Math::vec3 m_Velocity{};
		float m_Pitch{ 1.0f };
		float m_Gain{ 1.0f };
		bool m_IsLooping{ false };
		Assets::AssetRef<AudioBuffer> m_CurrentBuffer{};
	};

	class AudioSource
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		AudioSource();
		~AudioSource();
	public:
		//==============================
		// Getters/Setters
		//==============================
		SourceID& GetSourceID() { return m_SourceID; }
	private:
		//==============================
		// Internal Fields
		//==============================
		SourceID m_SourceID{ k_InvalidSourceID };
	};
}