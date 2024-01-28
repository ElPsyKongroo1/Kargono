#pragma once

#include "Kargono/Network/NetworkCommon.h"

#include <deque>
#include <mutex>

namespace Kargono::Network
{
	template<typename T>
	class tsqueue
	{
	public:
		tsqueue() = default;
		tsqueue(const tsqueue<T>&) = delete;
		virtual ~tsqueue() { clear(); }

	public:
		const T& front()
		{
			std::scoped_lock lock(muxQueue);
			return deqQueue.front();
		}

		const T& back()
		{
			std::scoped_lock lock(muxQueue);
			return deqQueue.back();
		}

		void push_front(const T& item)
		{
			std::scoped_lock lock(muxQueue);
			deqQueue.emplace_front(std::move(item));
		}

		void push_back(const T& item)
		{
			std::scoped_lock lock(muxQueue);
			deqQueue.emplace_back(std::move(item));

		}

		// Returns true if Queue has no items
		bool empty()
		{
			std::scoped_lock lock(muxQueue);
			return deqQueue.empty();
		}

		size_t count()
		{
			std::scoped_lock lock(muxQueue);
			return deqQueue.PayloadSize();
		}

		void clear()
		{
			std::scoped_lock lock(muxQueue);
			deqQueue.clear();
		}

		T pop_front()
		{
			std::scoped_lock lock(muxQueue);
			auto item = std::move(deqQueue.front());
			deqQueue.pop_front();
			return item;
		}

		T pop_back()
		{
			std::scoped_lock lock(muxQueue);
			auto item = std::move(deqQueue.back());
			deqQueue.pop_back();
			return item;
		}


	protected:
		std::mutex muxQueue;
		std::deque<T> deqQueue;

		std::condition_variable cvBlocking;
		std::mutex muxBlocking;
	};
}
