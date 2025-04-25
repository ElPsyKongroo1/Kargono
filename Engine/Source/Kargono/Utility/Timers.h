#pragma once

#include "Kargono/Core/Timestep.h"
#include "EventModule/ApplicationEvent.h"
#include "Kargono/Core/Base.h"

#include <chrono>
#include <cstdint>
#include <thread>
#include <functional>
#include <vector>
#include <mutex>
#include <atomic>

namespace Kargono
{
	using UpdateCount = uint64_t;
}

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

		static bool CloseAllTimers();

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
		static std::vector<Ref<AsyncBusyTimer>> s_AllBusyTimers;
		static std::mutex s_BusyTimerMutex;
	};

	class PassiveTimer
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		PassiveTimer(float waitTime, std::function<void()> function) : m_WaitTime{ waitTime }, m_Function{ function } {}
		~PassiveTimer() = default;
	public:
		static void CreateTimer(float waitTime, std::function<void()> function);
		static void OnUpdate(Timestep step);
	
	private:
		float m_WaitTime {0.0f};
		float m_ElapsedTime{0.0f};
		std::function<void()> m_Function {nullptr};
	private:
		static std::vector<PassiveTimer> s_AllPassiveTimers;
	};

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
		bool CheckForSingleUpdate();
		UpdateCount CheckForMultipleUpdates();

		//==============================
		// Getters/Setters
		//==============================
		// Config fields
		void SetConstantFrameTime(std::chrono::nanoseconds newFrameTime);
		std::chrono::nanoseconds GetConstantFrameTime();
		void SetConstantFrameTimeFloat(float newFrameTimeSeconds);
		float GetConstantFrameTimeFloat();
		// Accumulation fields
		UpdateCount GetUpdateCount();
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
		UpdateCount m_UpdateCount{ 0 };

		// Configuration data
		std::chrono::nanoseconds m_ConstantFrameTime{ 1'000 * 1'000 * 1'000 / 60 }; // 1/60th of a second
	};

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
		std::chrono::nanoseconds m_Accumulator{};
		// Configuration data
		std::chrono::nanoseconds m_UpdateDelta{ 1'000 * 1'000 * 1'000 / 60 /*1/60th sec*/ };
	};

}
