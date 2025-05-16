#include "kgpch.h"

#include "EventQueue.h"

#include "Kargono/Core/Base.h"

namespace Kargono::Events
{
	void EventQueue::Init(EventCallbackFn processQueueFunc)
	{
		KG_ASSERT(processQueueFunc);

		m_ProcessQueueFunc = processQueueFunc;
	}

	void EventQueue::SubmitEvent(Ref<Event> event)
	{
		// Obtain the event queue lock
		std::scoped_lock<std::mutex> lock(m_EventQueueMutex);

		// Add the event
		m_EventQueue.emplace_back(event);
	}

	void EventQueue::ClearQueue()
	{
		// Obtain the event queue lock
		std::scoped_lock<std::mutex> lock(m_EventQueueMutex);

		// Clear the queue
		m_EventQueue.clear();
	}

	void EventQueue::ProcessQueue()
	{
		KG_ASSERT(m_ProcessQueueFunc);

		std::vector<Ref<Event>> cachedEvents;

		// Cache a copy of the event queue to prevent loop invalidation
		{
			// Obtain the event queue lock
			std::scoped_lock<std::mutex> lock(m_EventQueueMutex);

			// Store the events in the cache
			cachedEvents = std::move(m_EventQueue);
			m_EventQueue.clear();
		}

		// Handle the event queue
		for (Ref<Event> event : cachedEvents)
		{
			m_ProcessQueueFunc(event.get());
		}
	}
}
