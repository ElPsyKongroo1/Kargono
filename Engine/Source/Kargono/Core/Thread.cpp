#include "kgpch.h"

#include "Thread.h"

namespace Kargono
{
	void KGThread::StartThread(std::function<void()> workFunction)
	{
		m_ThreadRunning = true;
		m_WorkFunction = workFunction;
		m_Thread = new std::thread(&KGThread::RunThread, this);
	}

	void KGThread::StopThread(bool withinThread)
	{
		if (withinThread)
		{
			m_ThreadRunning = false;
			return;
		}

		{
			std::lock_guard<std::mutex> lock(m_BlockThreadMutex);
			m_ThreadRunning = false;
			m_BlockThreadCV.notify_one();
		}

		m_Thread->join();
		delete m_Thread;
	}

	void KGThread::RunThread()
	{
		while (m_ThreadRunning)
		{
			{
				std::unique_lock<std::mutex> lock(m_BlockThreadMutex);

				// Change to the new work function
				if (m_NewWorkFunc)
				{
					m_WorkFunction = m_NewWorkFunc;
					m_NewWorkFunc = nullptr;
				}

				// Handle blocking the thread if necessary
				if (m_ThreadSuspended)
				{
					m_BlockThreadCV.wait(lock);
				}
			}

			m_WorkFunction();
		}
	}

	void KGThread::SuspendThread(bool withinThread)
	{
		if (withinThread)
		{
			m_ThreadSuspended = true;
			return;
		}

		std::lock_guard<std::mutex> lock(m_BlockThreadMutex);
		m_ThreadSuspended = true;
	}

	void KGThread::ResumeThread(bool withinThread)
	{
		if (withinThread)
		{
			m_ThreadSuspended = false;
			return;
		}

		std::lock_guard<std::mutex> lock(m_BlockThreadMutex);
		m_ThreadSuspended = false;
		m_BlockThreadCV.notify_one();
	}

	void KGThread::WaitOnThread()
	{
		m_Thread->join();
	}

	void KGThread::ChangeWorkFunction(std::function<void()> workFunction, bool withinThread)
	{
		if (withinThread)
		{
			// Set a new work function to be added
			m_NewWorkFunc = workFunction;
			return;
		}

		// Block the thread and swap the new thread function
		std::lock_guard<std::mutex> lock(m_BlockThreadMutex);
		m_NewWorkFunc = workFunction;
		m_BlockThreadCV.notify_one();
	}

	bool KGThread::IsRunning()
	{
		return m_ThreadRunning;
	}
}
