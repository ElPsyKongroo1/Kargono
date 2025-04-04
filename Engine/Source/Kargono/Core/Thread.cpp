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
			m_ThreadSuspended = false;
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

	void KGThread::ChangeWorkFunction(std::function<void()> workFunction)
	{
		if (m_ThreadRunning)
		{
			KG_WARN("Failed to change work function. Cannot change function while thread is running.");
			return;
		}

		m_WorkFunction = workFunction;
	}

	bool KGThread::IsRunning()
	{
		return m_ThreadRunning;
	}
}
