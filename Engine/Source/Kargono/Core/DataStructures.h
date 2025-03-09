#pragma once

#include <deque>
#include <mutex>

namespace Kargono
{
	template<typename T>
	class TSQueue
	{
	public:
		TSQueue() = default;
		TSQueue(const TSQueue<T>&) = delete;
		virtual ~TSQueue() { Clear(); }

	public:
		const T& GetFront()
		{
			std::scoped_lock lock(muxQueue);
			return deqQueue.front();
		}

		const T& GetBack()
		{
			std::scoped_lock lock(muxQueue);
			return deqQueue.back();
		}

		void PushFront(const T& item)
		{
			std::scoped_lock lock(muxQueue);
			deqQueue.emplace_front(std::move(item));
		}

		void PushBack(const T& item)
		{
			std::scoped_lock lock(muxQueue);
			deqQueue.emplace_back(std::move(item));
		}

		// Returns true if Queue has no items
		bool IsEmpty()
		{
			std::scoped_lock lock(muxQueue);
			return deqQueue.empty();
		}

		void Clear()
		{
			std::scoped_lock lock(muxQueue);
			deqQueue.clear();
		}

		T PopFront()
		{
			std::scoped_lock lock(muxQueue);
			auto item = std::move(deqQueue.front());
			deqQueue.pop_front();
			return item;
		}

		T PopBack()
		{
			std::scoped_lock lock(muxQueue);
			auto item = std::move(deqQueue.back());
			deqQueue.pop_back();
			return item;
		}


	private:
		std::mutex muxQueue;
		std::deque<T> deqQueue;

		std::condition_variable cvBlocking;
		std::mutex muxBlocking;
	};
}
