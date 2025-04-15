#include "kgpch.h"

#include "LoopTimer.h"

namespace Kargono::Utility
{
	void LoopTimer::InitializeTimer()
	{
		// Use chrono literals within this function scope
		using namespace std::chrono_literals;

		// Initialize all timepoints
		m_CurrentTime = std::chrono::high_resolution_clock::now();
		m_LastLoopTime = m_CurrentTime;

		// Initialize all accumulation data
		m_Timestep = 0ns;
		m_Accumulator = 0ns;
		m_UpdateCount = 0;
	}

	void LoopTimer::ResetAccumulator()
	{
		// Use chrono literals within this function scope
		using namespace std::chrono_literals;
		m_Accumulator = 0ns;
	}

	bool LoopTimer::CheckForUpdate()
	{
		// Update the timestep and accumulation
		m_CurrentTime = std::chrono::high_resolution_clock::now();
		m_Timestep = m_CurrentTime - m_LastLoopTime;
		m_LastLoopTime = m_CurrentTime;
		m_Accumulator += m_Timestep;

		// Check if an update is available based on current accumulation
		if (m_Accumulator < m_ConstantFrameTime)
		{
			return false;
		}

		// Handle an update
		m_Accumulator -= m_ConstantFrameTime;
		m_UpdateCount++;
		return true;
	}

	void LoopTimer::SetConstantFrameTime(std::chrono::nanoseconds newFrameTime)
	{
		m_ConstantFrameTime = newFrameTime;
	}

	void LoopTimer::SetConstantFrameTimeFloat(float newFrameTimeSeconds)
	{
		m_ConstantFrameTime = std::chrono::nanoseconds((long long)(newFrameTimeSeconds * 1'000'000'000));
	}

	std::chrono::nanoseconds LoopTimer::GetConstantFrameTime()
	{
		return m_ConstantFrameTime;
	}

	float LoopTimer::GetConstantFrameTimeFloat()
	{
		return std::chrono::duration<float>(m_ConstantFrameTime).count();
	}

	uint64_t LoopTimer::GetUpdateCount()
	{
		return m_UpdateCount;
	}
}
