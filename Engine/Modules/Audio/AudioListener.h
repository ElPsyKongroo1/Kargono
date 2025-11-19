#pragma once

#include "Kargono/Math/MathAliases.h"

#include <cstdint>

namespace Kargono::Audio
{

	using ListenerID = uint32_t;
	constexpr ListenerID k_InvalidListenerID{ 0 }; // TODO: Maybe change to upperbound??

	struct AudioListenerSpecification
	{
		Math::vec3 m_Position{};
		Math::vec3 m_Velocity{};
		Math::vec3 m_Forward{ 1.0f, 0.0f, 0.0f };
		Math::vec3 m_Up{};
	};

	class AudioListener // TODO: IMPLEMENT LATER
	{
	public:
		//==============================
		// Constructors and Destructors
		//==============================
		AudioListener() = default;
		~AudioListener() = default;
	private:
		//==============================
		// Internal Fields
		//==============================
		ListenerID m_ListenerID{ k_InvalidListenerID };
	};
}