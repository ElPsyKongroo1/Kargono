#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Events/Event.h"

#include <vector>
#include <mutex>
#include <memory>
#include <functional>

namespace Kargono::Events
{
	class EventQueue
	{
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		EventQueue() = default;
		~EventQueue() = default;

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
		std::vector<Ref<Event>> m_EventQueue{};
		std::mutex m_EventQueueMutex{};
		EventCallbackFn m_ProcessQueueFunc{ nullptr };
	};
}
