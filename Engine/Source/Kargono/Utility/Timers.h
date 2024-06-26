#pragma once

#include "Kargono/Core/Timestep.h"
#include "Kargono/Events/ApplicationEvent.h"
#include "Kargono/Core/Base.h"

#include <chrono>
#include <thread>
#include <functional>
#include <vector>
#include <mutex>
#include <atomic>


namespace Kargono::Utility
{
	class ScopedTimer
	{
	public:
		ScopedTimer()
		{
			Reset();
		}

		void Reset()
		{
			m_Start = std::chrono::high_resolution_clock::now();
		}

		float Elapsed()
		{
			return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_Start).count() * 0.001f * 0.001f * 0.001f;
		}

		float ElapsedMillis()
		{
			return Elapsed() * 1000.0f;
		}

		float ElapsedMicro()
		{
			return Elapsed() * 1000000.0f;
		}

	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
	};

	class AsyncBusyTimer
	{
	public:
		static void CreateTimer(float waitTime, std::function<void()> function);

		static void CreateRecurringTimer(float waitTime, uint32_t reoccurCount, std::function<void()> reoccurFunction,
			std::function<void()> terminationFunction = nullptr);

		static void CleanUpClosedTimers();

		static void CloseAllTimers();

	public:
		AsyncBusyTimer(float waitTime, std::function<void()> function);

		AsyncBusyTimer(float waitTime, uint32_t reoccurCount, std::function<void()> reoccurFunction,
			std::function<void()> terminationFunction);

		~AsyncBusyTimer();

	public:
		void ForceStop() { m_ForceStop = true; }
	private:
		void Wait();
		void ReoccurWait();
	private:
		std::function<void()> m_TerminationFunction { nullptr };
		std::function<void()> m_ReoccurFunction { nullptr };
		float m_WaitTime;
		float m_ElapsedTime;
		uint32_t m_ReoccurCount{ 0 };
		std::thread m_TimerThread;
		bool m_Done {false};
		std::atomic<bool> m_ForceStop {false};
	private:
		static std::vector<Ref<AsyncBusyTimer>> s_AllTimers;
		static std::mutex s_BusyTimerMutex;
	};

}
