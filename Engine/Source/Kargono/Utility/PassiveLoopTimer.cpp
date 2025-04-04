#include "kgpch.h"

#include "PassiveLoopTimer.h"

namespace Kargono::Utility
{
	void PassiveLoopTimer::InitializeTimer(std::chrono::nanoseconds updateDelta)
	{
		m_UpdateDelta = updateDelta;
		InitializeTimer();
	}

	void PassiveLoopTimer::InitializeTimer(float updateDeltaSeconds)
	{
		InitializeTimer(std::chrono::nanoseconds((long long)(updateDeltaSeconds * 1'000'000'000)));
	}

	void PassiveLoopTimer::InitializeTimer()
	{
		using namespace std::chrono_literals;

		m_Accumulator = 0ns;
	}

	bool PassiveLoopTimer::CheckForUpdate(std::chrono::nanoseconds timestep)
	{
		m_Accumulator += timestep;

		if (m_Accumulator < m_UpdateDelta)
		{
			return false;
		}

		m_Accumulator -= m_UpdateDelta;
		return true;
	}

	void PassiveLoopTimer::SetUpdateDelta(std::chrono::nanoseconds newFrameTime)
	{
		m_UpdateDelta = newFrameTime;
	}

	void PassiveLoopTimer::SetUpdateDeltaFloat(float newFrameTimeSeconds)
	{
		m_UpdateDelta = std::chrono::nanoseconds((long long)(newFrameTimeSeconds * 1'000'000'000));
	}
}
