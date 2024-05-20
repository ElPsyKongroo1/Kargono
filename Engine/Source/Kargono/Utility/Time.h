#pragma once
#include <cstdint>

namespace Kargono::Utility
{
	//============================================================
	// Time Class
	//============================================================
	// This class provides static functions dealing with time such as
	//		getting the current time values in seconds since the application
	//		started.
	class Time
	{
	public:
		// This function returns the time since that application has started
		//		in seconds (Technically, the time since GLFW was initialized).
		static float GetTime();
		static void GetTimeFromMilliseconds(uint64_t initialMilliseconds, uint64_t& hours, uint64_t& minutes, uint64_t& seconds, uint64_t& milliseconds);
		static std::string GetStringFromMilliseconds(uint64_t initialMilliseconds);
		static std::string GetStringFromSeconds(uint64_t initialSeconds);
	};
}
