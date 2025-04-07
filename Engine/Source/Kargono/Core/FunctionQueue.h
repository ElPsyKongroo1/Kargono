#pragma once

#include <vector>
#include <functional>

class FunctionQueue
{
public:
	//=========================
	// Constructor/Destructor
	//=========================
	FunctionQueue() = default;
	~FunctionQueue() = default;
public:
	//=========================
	// Modify Queue
	//=========================
	void SubmitFunction(const std::function<void()>& function);
	void ClearQueue();

	//=========================
	// Submit Queue
	//=========================
	void ProcessQueue();
private:
	//=========================
	// Internal Fields
	//=========================
	std::vector<std::function<void()>> m_FunctionQueue{};
	std::mutex m_FunctionQueueMutex{};
};
