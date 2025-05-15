#pragma once
#include <cstdint>

namespace Kargono::Utility
{
	//============================================================
	// Time Class
	//============================================================
	// TODO: NOTE THAT THIS SHOULD DEPEND ON THE WINDOWING CONTEXT
	class Time
	{
	public:
		//==============================
		// Get Time
		//==============================
		static float GetTime();
		
	};

	class TimeConversions
	{
	public:
		//==============================
		// Convert Milliseconds -> Hours/Minutes/Seconds/Milli
		//==============================
		static void GetTimeFromMilliseconds(uint64_t initialMilliseconds, uint64_t& hours, uint64_t& minutes, uint64_t& seconds, uint64_t& milliseconds);
		//==============================
		// ToString Functions
		//==============================
		static std::string GetStringFromMilliseconds(uint64_t initialMilliseconds);
		static std::string GetStringFromSeconds(uint64_t initialSeconds);
	};
}
