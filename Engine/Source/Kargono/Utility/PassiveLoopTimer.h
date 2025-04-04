#pragma once
#include "chrono"

namespace Kargono::Utility
{
	class PassiveLoopTimer
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		PassiveLoopTimer() = default;
		~PassiveLoopTimer() = default;

		//==============================
		// Lifecycle Functions
		//==============================
		// Reset the timer
		void InitializeTimer(std::chrono::nanoseconds updateDelta);
		void InitializeTimer(float updateDeltaSeconds);
		void InitializeTimer();
		// Move the timer forward
		bool CheckForUpdate(std::chrono::nanoseconds timestep);

		//==============================
		// Getters/Setters
		//==============================
		// Manage constant frame time
		void SetUpdateDelta(std::chrono::nanoseconds newFrameTime);
		void SetUpdateDeltaFloat(float newFrameTimeSeconds);
	private:
		//==============================
		// Internal Fields
		//==============================
		// Accumulation data
		std::chrono::nanoseconds m_Accumulator;

		// Configuration data
		std::chrono::nanoseconds m_UpdateDelta{ 1'000 * 1'000 * 1'000 / 60 };
	};
}
