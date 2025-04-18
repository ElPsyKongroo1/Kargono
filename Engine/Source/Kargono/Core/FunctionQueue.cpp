#include "kgpch.h"
#include "FunctionQueue.h"

void FunctionQueue::SubmitFunction(const std::function<void()>& function)
{
	// Obtain the function queue lock
	std::scoped_lock<std::mutex> lock(m_FunctionQueueMutex);

	// Add the function
	m_FunctionQueue.emplace_back(function);
}

void FunctionQueue::ClearQueue()
{
	// Obtain the function queue lock
	std::scoped_lock<std::mutex> lock(m_FunctionQueueMutex);

	m_FunctionQueue.clear();
}

void FunctionQueue::ProcessQueue()
{
	std::vector<std::function<void()>> localFunctionCache;

	// Ensure the function queue doesn't become invalidated if one of the functions modifies the queue
	// inside the loop
	{
		localFunctionCache.resize(m_FunctionQueue.size());

		// Obtain the function queue lock
		std::scoped_lock<std::mutex> lock(m_FunctionQueueMutex);

		// Move the event queue into a local variable and reset the queue
		localFunctionCache = std::move(m_FunctionQueue);

		// Reset the queue
		m_FunctionQueue.clear();
	}


	// Call all of the functions in the queue
	for (std::function<void()>& func : localFunctionCache)
	{
		func();
	}
}
