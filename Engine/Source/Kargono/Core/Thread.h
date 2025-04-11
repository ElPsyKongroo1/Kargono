#pragma once
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>

namespace Kargono
{
	class KGThread
	{
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		void StartThread(std::function<void()> workFunction);
		void StopThread(bool withinThread = false);
		void RunThread();

		//==============================
		// Manage Thread
		//==============================
		void SuspendThread(bool withinThread = false);
		void ResumeThread(bool withinThread = false);
		void WaitOnThread();
		void ChangeWorkFunction(std::function<void()> workFunction, bool withinThread = false);

		//==============================
		// Query Thread
		//==============================
		bool IsRunning();
	private:
		//==============================
		// Internal Fields
		//==============================
		// Thread and running function
		std::thread* m_Thread{ nullptr };
		std::function<void()> m_WorkFunction{ nullptr };
		std::function<void()> m_NewWorkFunc{ nullptr };
		// Management fields
		std::atomic<bool> m_ThreadRunning{ false };
		std::mutex m_BlockThreadMutex{};
		std::condition_variable m_BlockThreadCV{};
		bool m_ThreadSuspended{ false };
	};
}
