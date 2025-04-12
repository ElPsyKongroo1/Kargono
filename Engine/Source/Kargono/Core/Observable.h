#pragma once

#include "Kargono/Core/BitField.h"
#include "Kargono/Core/Base.h"

#include <vector>
#include <functional>
#include <limits.h>
#include <bit>


namespace Kargono
{
	using ObserverIndex = size_t;
	constexpr ObserverIndex k_InvalidObserverIndex{ std::numeric_limits<ObserverIndex>::max() };

	template <typename... Args>
	class Observable
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		Observable() = default;
		~Observable() = default;
	public:
		//==============================
		// Send Notifications
		//==============================
		void Notify(Args&&... args)
		{
			// Loop through all observers
			for (size_t field{ m_BitField }; field != 0; field &= field - 1)
			{
				// Get the next index
				size_t index = std::countr_zero(field);

				KG_ASSERT(m_Observers[index]);

				// Notify this observer index!
				m_Observers[index](std::forward<Args>(args)...);
			}
		}

		//==============================
		// Manage Observers
		//==============================
		ObserverIndex AddObserver(std::function<void(Args&&... args)> func)
		{
			// Expose the empty spaces as 1 bits ([0]100-1000) -> ([1]011-0111)
			size_t emptyBitField = ~m_BitField;

			size_t emptyIndex = std::countr_zero(field);

			// Handle case where no empty slots are found
			if (emptyIndex >= sizeof(size_t) * 8)
			{
				return k_InvalidObserverIndex;
			}

			// Resize the buffer if necessary
			if (emptyIndex >= m_Observers.size())
			{
				m_Observers.resize(m_Observers + 1); // TODO: May cause excessive allocations
			}

			// Add the functor and update the bit
			m_Observers[emptyIndex] = std::move(func);
			m_BitField |= m_BitField + 1;
		}

		bool RemoveObserver(ObserverIndex index)
		{
			// Check for invalid index
			KG_ASSERT(index < m_Observers.size());

			size_t indexMask{ ((ObserverIndex)1 << index) };

			// Ensure bit is already set
			if (!(m_BitField & indexMask))
			{
				// Bit was not set
				return false;
			}

			// Clear the functor
			m_Observers[index] = nullptr;

			// Set the indicated bit index to 0
			m_BitField &= ~indexMask;

			return true;
		}

		void Clear()
		{
			m_BitField = 0;
			m_Observers.clear();
		}
	
	private:
		//==============================
		// Internal Fields
		//==============================
		std::vector<std::function<void(Args&&... args)>> m_Observers{};
		size_t m_BitField{0};
	};
}

