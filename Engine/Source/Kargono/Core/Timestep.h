#pragma once
#include <cstdint>

namespace Kargono
{
	class Timestep
	{
	public:
		//==============================
		// Constructor/Destructor
		//==============================
		Timestep(float time = 0.0f) : m_Time(time) {}

		//==============================
		// Getters/Setters
		//==============================
		operator float() const 
		{ 
			return m_Time; 
		}

		float GetSeconds() const 
		{ 
			return m_Time;
		}
		float GetMilliseconds() const 
		{ 
			return m_Time * 1000.0f;
		}
		int32_t GetMillisecondsInt() const
		{
			return (int32_t)(m_Time * 1000.0f);
		}
	private:
		//==============================
		// Internal Fields
		//==============================
		float m_Time;
	};
}
