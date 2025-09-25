#pragma once

#include "Kargono/Math/MathAliases.h"

namespace Kargono::Particles
{
	struct Particle
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		Particle() = default;
		~Particle() = default;
	public:
		//==============================
		// Public Fields
		//==============================
		// Particle transform 
		Math::vec3 m_Position;
		Math::vec3 m_Size;
		Math::vec3 m_Rotation;
		// Particle physics
		Math::vec3 m_Velocity;
		// Particle state
		bool m_Active{ false };
	private:
		//==============================
		// Internal Fields
		//==============================
		// Particle lifetime information
		float m_StartTime;
		float m_EndTime;
	private:
		//==============================
		// Friend(s)
		//==============================
		friend class ParticleContext;
	};
}