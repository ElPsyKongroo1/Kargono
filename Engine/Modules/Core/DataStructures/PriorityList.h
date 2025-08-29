#pragma once

#include <limits>
#include <cstdint>
#include <concepts>
#include <vector>

namespace Kargono
{
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