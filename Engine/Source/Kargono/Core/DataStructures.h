#pragma once

#include <deque>
#include <mutex>
#include <vector>
#include <optional>
#include <cstdint>
#include <type_traits>
#include <concepts>
#include <algorithm>
#include <numeric>

#include <Kargono/Core/Iterator.h>
#include "Kargono/Core/Base.h"

namespace Kargono
{
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

	using PriorityListIndex = size_t;
	constexpr PriorityListIndex k_InvalidPriorityListIndex
	{ 
		std::numeric_limits<PriorityListIndex>::max() 
	};

	template<typename t_Type, std::unsigned_integral t_PriorityInt = uint8_t>
	class PriorityList
	{
	public:
		//==============================
		// Add Item
		//==============================
		void AddItem(const t_Type& item, t_PriorityInt priority)
		{
			m_Priorities.push_back(priority);
			m_DataList.push_back(item);
		}

		//==============================
		// Find Item
		//==============================
		// NOTE, indexes are not persistent. They change after sorting!!!!
		PriorityListIndex FindItem(std::function<bool(const t_Type&)> findCallback) const
		{
			PriorityListIndex i{ 0 };

			for (const t_Type& item : m_DataList)
			{
				if (findCallback(item))
				{
					return i;
				}
			}

			return k_InvalidPriorityListIndex;
		}

		//==============================
		// Remove Item
		//==============================
		[[nodiscard]] bool RemoveIndex(PriorityListIndex index)
		{
			KG_ASSERT(m_DataList.size() == m_Priorities.size());

			// Ensure the index is valid
			if (index >= m_DataList.size())
			{
				return false;
			}

			// Remove item and priority from internal lists
			m_DataList.erase(m_DataList.begin() + index);
			m_Priorities.erase(m_Priorities.begin() + index);

			return true;
		}

		//==============================
		// Clear
		//==============================
		void Clear()
		{
			m_DataList.clear();
			m_Priorities.clear();
		}
	public:
		//==============================
		// Sort By Priority
		//==============================
		void SortList()
		{
			// Note: I know this is expensive. I'm optimizing for cache friendliness
			// ...not insertion/sort speed.

			KG_ASSERT(m_DataList.size() == m_Priorities.size());
			size_t listSize{ m_DataList.size() };

			if (listSize == 0)
			{
				return;
			}

			// Create a vector of ascending indices 0 - (size - 1)
			std::vector<size_t> indices(listSize);
			std::iota(indices.begin(), indices.end(), 0);

			// Sort the indices by priority
			std::sort(indices.begin(), indices.end(),
				[this](size_t a, size_t b)
				{
					return m_Priorities[a] < m_Priorities[b];
				});

			// Create sorted versions of the vectors
			std::vector<t_Type> sortedItems;
			std::vector<t_PriorityInt> sortedPriorities;

			sortedItems.reserve(listSize);
			sortedPriorities.reserve(listSize);

			for (size_t index : indices)
			{
				sortedItems.push_back(std::move(m_DataList[index]));
				sortedPriorities.push_back(m_Priorities[index]);
			}

			// Replace original with sorted
			m_DataList = std::move(sortedItems);
			m_Priorities = std::move(sortedPriorities);
		}

		//==============================
		// Allow For-loop
		//==============================
		std::vector<t_Type>::iterator begin()
		{
			return m_DataList.begin();
		}
		std::vector<t_Type>::iterator end()
		{
			return m_DataList.end();
		}

		std::vector<t_Type>::const_iterator begin() const
		{
			return m_DataList.begin();
		}
		std::vector<t_Type>::const_iterator end() const
		{
			return m_DataList.end();
		}

	private:
		//==============================
		// Internal Fields
		//==============================
		std::vector<t_Type> m_DataList{};
		std::vector<t_PriorityInt> m_Priorities{};
	};
	
}
