#include "kgpch.h"

#include "Kargono/Utility/Timers.h"

namespace Kargono::Utility
{
	std::vector<Ref<AsyncBusyTimer>> AsyncBusyTimer::s_AllBusyTimers {};
	std::vector<PassiveTimer> PassiveTimer::s_AllPassiveTimers {};
	std::mutex AsyncBusyTimer::s_BusyTimerMutex {};

	void AsyncBusyTimer::CreateTimer(float waitTime, std::function<void()> function)
	{
		std::scoped_lock lock(s_BusyTimerMutex);
		s_AllBusyTimers.push_back(CreateRef<AsyncBusyTimer>(waitTime, function));
		// TODO: Not amazing solution, but passively cleans up timers
		CleanUpClosedTimers();
	}
	void AsyncBusyTimer::CreateRecurringTimer(float waitTime, uint32_t reoccurCount, std::function<void()> reoccurFunction, std::function<void()> terminationFunction)
	{
		std::scoped_lock lock(s_BusyTimerMutex);
		s_AllBusyTimers.push_back(CreateRef<AsyncBusyTimer>(waitTime, reoccurCount, reoccurFunction, terminationFunction));
		// TODO: Not amazing solution, but passively cleans up timers
		CleanUpClosedTimers();
	}
	void AsyncBusyTimer::CleanUpClosedTimers()
	{
		auto iterator = std::remove_if(s_AllBusyTimers.begin(), s_AllBusyTimers.end(), [&](Ref<AsyncBusyTimer> timer)
			{
				if (timer->m_Done)
				{
					return true;
				}
				return false;
			});

		s_AllBusyTimers.erase(iterator, s_AllBusyTimers.end());
	}

	bool AsyncBusyTimer::CloseAllTimers()
	{
		std::scoped_lock lock(s_BusyTimerMutex);

		for (Ref<AsyncBusyTimer> timer : s_AllBusyTimers)
		{
			timer->ForceStop();
		}

		for(Ref<AsyncBusyTimer> timer : s_AllBusyTimers)
		{
			if (timer->m_TimerThread.joinable())
			{
				timer->m_TimerThread.join();
			}
		}
		s_AllBusyTimers.clear();
		if (s_AllBusyTimers.size() != 0)
		{
			KG_WARN("Busy timers not closed successfully");
			return false;
		}
		return true;
	}

	AsyncBusyTimer::AsyncBusyTimer(float waitTime, std::function<void()> function)
		: m_TerminationFunction(function), m_WaitTime(waitTime), m_ElapsedTime(0)
	{
		m_TimerThread = std::thread(&AsyncBusyTimer::Wait, this);
	}
	AsyncBusyTimer::AsyncBusyTimer(float waitTime, uint32_t reoccurCount, std::function<void()> reoccurFunction, std::function<void()> terminationFunction)
		: m_TerminationFunction(terminationFunction), m_WaitTime(waitTime), m_ElapsedTime(0), m_ReoccurCount(reoccurCount), m_ReoccurFunction(reoccurFunction)
	{
		m_TimerThread = std::thread(&AsyncBusyTimer::ReoccurWait, this);
	}
	AsyncBusyTimer::~AsyncBusyTimer()
	{
		if (m_TimerThread.joinable())
		{
			m_TimerThread.join();
		}
	}
	void AsyncBusyTimer::Wait()
	{
		using namespace std::chrono_literals;

		std::chrono::time_point<std::chrono::high_resolution_clock> currentTime = std::chrono::high_resolution_clock::now();
		std::chrono::time_point<std::chrono::high_resolution_clock> lastCycleTime = currentTime;
		std::chrono::nanoseconds timeStep{ 0 };
		std::chrono::nanoseconds accumulator{ 0 };
		std::chrono::nanoseconds waitTime { static_cast<uint64_t>(m_WaitTime * 1'000 * 1'000 * 1'000) };

		while (waitTime > accumulator && !m_ForceStop)
		{
			currentTime = std::chrono::high_resolution_clock::now();
			timeStep = currentTime - lastCycleTime;
			lastCycleTime = currentTime;
			accumulator += timeStep;
		}
		if (m_ForceStop)
		{
			m_Done = true;
			return;
		}

		m_TerminationFunction();
		m_Done = true;
	}

	void AsyncBusyTimer::ReoccurWait()
	{
		using namespace std::chrono_literals;

		std::chrono::time_point<std::chrono::high_resolution_clock> currentTime = std::chrono::high_resolution_clock::now();
		std::chrono::time_point<std::chrono::high_resolution_clock> lastCycleTime = currentTime;
		std::chrono::nanoseconds timeStep{ 0 };
		std::chrono::nanoseconds accumulator{ 0 };
		std::chrono::nanoseconds waitTime { static_cast<uint64_t>(m_WaitTime * 1'000 * 1'000 * 1'000) };

		for (uint32_t iteration {0}; iteration < m_ReoccurCount; iteration++)
		{
			accumulator = 0ns;
			currentTime = std::chrono::high_resolution_clock::now();
			lastCycleTime = currentTime;

			while (waitTime > accumulator && !m_ForceStop)
			{
				currentTime = std::chrono::high_resolution_clock::now();
				timeStep = currentTime - lastCycleTime;
				lastCycleTime = currentTime;
				accumulator += timeStep;
			}

			m_ReoccurFunction();
		}

		while (waitTime > accumulator && !m_ForceStop)
		{
			currentTime = std::chrono::high_resolution_clock::now();
			timeStep = currentTime - lastCycleTime;
			lastCycleTime = currentTime;
			accumulator += timeStep;
		}
		if (m_ForceStop)
		{
			m_Done = true;
			return;
		}

		if (m_TerminationFunction)
		{
			m_TerminationFunction();
		}
		
		m_Done = true;
	}
	void PassiveTimer::CreateTimer(float waitTime, std::function<void()> function)
	{
		s_AllPassiveTimers.push_back({ waitTime, function });
	}
	void PassiveTimer::OnUpdate(Timestep step)
	{
		std::vector<uint32_t> timersToRemove{};
		uint32_t iteration{ 0 };
		for (PassiveTimer& timer : s_AllPassiveTimers)
		{
			timer.m_ElapsedTime += step;
			if (timer.m_ElapsedTime > timer.m_WaitTime)
			{
				timer.m_Function();
				timersToRemove.push_back(iteration);
			}
			iteration++;
		}

		for (auto it = timersToRemove.rbegin(); it != timersToRemove.rend(); ++it)
		{
			s_AllPassiveTimers.erase(s_AllPassiveTimers.begin() + *it);
		}
	}

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
