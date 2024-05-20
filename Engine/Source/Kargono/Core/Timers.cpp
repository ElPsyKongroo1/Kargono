#include "kgpch.h"

#include "Kargono/Core/Timers.h"

namespace Kargono::Timers
{
	std::vector<Ref<AsyncBusyTimer>> AsyncBusyTimer::s_AllTimers {};
	std::mutex AsyncBusyTimer::s_BusyTimerMutex {};

	void AsyncBusyTimer::CreateTimer(float waitTime, std::function<void()> function)
	{
		std::scoped_lock lock(s_BusyTimerMutex);
		s_AllTimers.push_back(CreateRef<AsyncBusyTimer>(waitTime, function));
		// TODO: Not amazing solution, but passively cleans up timers
		CleanUpClosedTimers();
	}
	void AsyncBusyTimer::CreateRecurringTimer(float waitTime, uint32_t reoccurCount, std::function<void()> reoccurFunction, std::function<void()> terminationFunction)
	{
		std::scoped_lock lock(s_BusyTimerMutex);
		s_AllTimers.push_back(CreateRef<AsyncBusyTimer>(waitTime, reoccurCount, reoccurFunction, terminationFunction));
		// TODO: Not amazing solution, but passively cleans up timers
		CleanUpClosedTimers();
	}
	void AsyncBusyTimer::CleanUpClosedTimers()
	{
		auto iterator = std::remove_if(s_AllTimers.begin(), s_AllTimers.end(), [&](Ref<AsyncBusyTimer> timer)
			{
				if (timer->m_Done)
				{
					return true;
				}
				return false;
			});

		s_AllTimers.erase(iterator, s_AllTimers.end());
	}

	void AsyncBusyTimer::CloseAllTimers()
	{
		std::scoped_lock lock(s_BusyTimerMutex);

		for (auto& timer : s_AllTimers)
		{
			timer->ForceStop();
		}

		for(auto& timer : s_AllTimers)
		{
			if (timer->m_TimerThread.joinable())
			{
				timer->m_TimerThread.join();
			}
		}
		s_AllTimers.clear();
		KG_INFO("All async timers closed successfully!");
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
}
