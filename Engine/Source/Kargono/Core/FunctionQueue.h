#pragma once

#include <vector>
#if 0
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
	// Lifecycle Functions
	//=========================
	void Init(EventCallbackFn processQueueFunc);
public:
	//=========================
	// Modify Queue
	//=========================
	void SubmitEvent(Ref<Event> event);
	void ClearQueue();

	//=========================
	// Submit Queue
	//=========================
	void ProcessQueue();
private:
	//=========================
	// Internal Fields
	//=========================
	std::vector<Ref<Event>> m_FunctionQueue{};
	std::mutex m_FunctionQueueMutex{};
	EventCallbackFn m_ProcessQueueFunc{ nullptr };
};

#endif
