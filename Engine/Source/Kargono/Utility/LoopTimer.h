#pragma once
#include <chrono>

namespace Kargono::Utility
{
	class LoopTimer
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		LoopTimer() = default;
		~LoopTimer() = default;

		//==============================
		// Lifecycle Functions
		//==============================
		// Reset the timer
		void InitializeTimer();
		void ResetAccumulator();
		// Move the timer context forward
		bool CheckForUpdate();

		//==============================
		// Getters/Setters
		//==============================
		// Manage constant frame time
		void SetConstantFrameTime(std::chrono::nanoseconds newFrameTime);
		std::chrono::nanoseconds GetConstantFrameTime();
		void SetConstantFrameTimeFloat(float newFrameTimeSeconds);
		float GetConstantFrameTimeFloat();

		uint64_t GetUpdateCount();
	private:
		//==============================
		// Internal Fields
		//==============================
		// Timepoints (for calculating time-step)
		std::chrono::time_point<std::chrono::high_resolution_clock> m_CurrentTime;
		std::chrono::time_point<std::chrono::high_resolution_clock> m_LastLoopTime;

		// Accumulating data
		std::chrono::nanoseconds m_Timestep{ 0 };
		std::chrono::nanoseconds m_Accumulator{ 0 };
		uint64_t m_UpdateCount{ 0 };

		// Configuration data
		std::chrono::nanoseconds m_ConstantFrameTime{ 1'000 * 1'000 * 1'000 / 60 }; // 1/60th of a second
	};
}
