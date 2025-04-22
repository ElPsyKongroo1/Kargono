#pragma once

#include <deque>
#include <mutex>
#include <vector>
#include <optional>
#include <cstdint>
#include <type_traits>
#include <concepts>

#include <Kargono/Core/Iterator.h>
#include "Kargono/Core/Base.h"

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


	template<typename T, std::unsigned_integral IndexType = size_t>
	struct EmplaceResult
	{
		IndexType m_ArrayIndex{ 0 };
		T& m_Value;
	};

	// TODO: Maybe use a data buffer along w/ placement new to avoid unnecessary constructor calls
	// Note that this data structure does not manage the destruction of objects
	template<typename T, std::unsigned_integral IndexType = size_t>
	class SparseArray
	{

	public:
		//==============================
		// Constructors/Destructors
		//==============================
		SparseArray() = default;
		SparseArray(IndexType arraySize)
		{
			SetMaxSize(arraySize);
		}
		~SparseArray() = default;
	public:
		//==============================
		// Query State
		//==============================
		IndexType GetCount() const
		{
			return m_Count;
		}

		IndexType GetCapacity() const
		{
			return m_Capacity;
		}

		std::vector<IndexType> GetActiveIndices()
		{
			// TODO: Note that this function is quite expensive
			std::vector<IndexType> returnIndices{};
			returnIndices.reserve(m_Count);
			
			// Store all active indices
			IndexType i{ 0 };
			for (bool active : m_ActiveArray)
			{
				if (active)
				{
					returnIndices.push_back(i);
				}
				i++;
			}

			return returnIndices;
		}

		//==============================
		// Modify State
		//==============================
		Expected<EmplaceResult<T, IndexType>> EmplaceLowest()
		{
			IndexType index{ 0 };

			// Check for empty slot
			for (bool active : m_ActiveArray)
			{
				if (!active)
				{
					break;
				}
				index++;
			}

			// Check for failure to find slot
			if (index >= m_Capacity)
			{
				return {};
			}

			// Return the found object
			m_Count++;
			m_ActiveArray[index] = true;

			return EmplaceResult(index, m_Array[index]);
		}

		bool Remove(IndexType index)
		{
			KG_ASSERT(index < m_Capacity);

			// Ensure the index is active
			if (!m_ActiveArray[index])
			{
				return false;
			}

			KG_ASSERT(m_Count > 0);

			// Clear the index
			m_Count--;
			m_ActiveArray[index] = false;
			return true;
		}

		void SetMaxSize(IndexType newSize)
		{
			m_Capacity = newSize;
			m_Array.resize(m_Capacity);
			m_ActiveArray.resize(m_Capacity);
		}

		void Clear()
		{
			// TODO: Find a better way 
			for (IndexType i{ 0 }; i < m_Capacity; i++)
			{
				m_ActiveArray[i] = false;
			}
		}
		//==============================
		// Operator Overloads
		//==============================
		T& operator[](IndexType index)
		{
			KG_ASSERT(index < m_Capacity);
			KG_ASSERT(m_ActiveArray[index]);

			return m_Array[index];
		}

		//==============================
		// Iterators API
		//==============================
		struct SparseArrayIterator : public Iterator<SparseArrayIterator, SparseArray<T, IndexType>, T>
		{
			// Constructor to initialize the iterator
			SparseArrayIterator(SparseArray* container, T* dataPtr)
				: Iterator<SparseArrayIterator, SparseArray<T, IndexType>, T>(container, dataPtr) {}

			bool Compare(T* dataPtr)
			{
				// Ensure the pointer is within the bounds of the array
				if ((uintptr_t)dataPtr < (uintptr_t)this->m_ContainerPtr->m_Array.data() ||
					(uintptr_t)dataPtr >= (uintptr_t)this->m_ContainerPtr->m_Array.data() + this->m_ContainerPtr->m_Capacity * sizeof(T))
				{
					return false;
				}

				// Calculate the index from the pointer
				size_t index{ (size_t)(dataPtr - (T*)this->m_ContainerPtr->m_Array.data()) };
				// Check if the index is active
				return !this->m_ContainerPtr->m_ActiveArray[index];
			}
		};

		SparseArrayIterator begin()
		{
			// Find the first valid index or the end
			size_t i{0};
			while (i < m_Capacity)
			{
				if (m_ActiveArray[i])
				{
					break;
				}
				i++;
			}

			// Return the first valid index
			return { this, (T*)m_Array.data() + i };
		}

		SparseArrayIterator end()
		{
			return { this, (T*)m_Array.data() + m_Capacity };
		}

	private:
		//==============================
		// Internal Fields
		//==============================
		IndexType m_Capacity{ 0 };
		IndexType m_Count{ 0 };
		std::vector<T> m_Array{};
		std::vector<bool> m_ActiveArray{};

	private:
		friend struct SparseArrayIterator;
	};

	// TODO: Add SparseSet
	
}
