#pragma once

#include "Kargono/Core/Base.h"

#include <vector>
#include <functional>
#include <limits.h>
#include <bit>

namespace Kargono
{

	template <typename... Args>
	class SingleNotifier
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		SingleNotifier() = default;
		~SingleNotifier() = default;
	public:
		//==============================
		// Send Notifications
		//==============================
		void Notify(Args... args)
		{
			KG_ASSERT(m_Callback);
			m_Callback(std::forward<Args>(args)...);
		}

		//==============================
		// Manage Callback
		//==============================
		void SetCallback(std::function<void(Args... args)> func)
		{
			// Add the functor and update the bit
			m_Callback = std::move(func);
		}

		void ClearCallback()
		{
			m_Callback = nullptr;
		}

	private:
		//==============================
		// Internal Fields
		//==============================
		std::function<void(Args... args)> m_Callback;
	};

	using ListenerIndex = size_t;
	constexpr ListenerIndex k_InvalidListenerIndex{ std::numeric_limits<ListenerIndex>::max() };

	template <typename... Args>
	class MultiNotifier
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		MultiNotifier() = default; 
		~MultiNotifier() = default;
	public:
		//==============================
		// Send Notifications
		//==============================
		void Notify(Args... args)
		{
			// Loop through all observers
			for (size_t field{ m_BitField }; field != 0; field &= field - 1)
			{
				// Get the next index
				size_t index = std::countr_zero(field);

				KG_ASSERT(m_Listeners[index]);

				// Notify this observer index!
				m_Listeners[index](std::forward<Args>(args)...);
			}
		}

		//==============================
		// Manage Observers
		//==============================
		ListenerIndex AddObserver(std::function<void(Args... args)> func)
		{
			// Get the index of the first empty space
			size_t emptyIndex = std::countr_zero(~m_BitField);

			// Handle case where no empty slots are found
			if (emptyIndex >= sizeof(size_t) * 8)
			{
				return k_InvalidListenerIndex;
			}

			// Resize the buffer if necessary
			if (emptyIndex >= m_Listeners.size())
			{
				m_Listeners.resize(emptyIndex + 1); // TODO: May cause excessive allocations
			}

			// Add the functor and update the bit
			m_Listeners[emptyIndex] = std::move(func);
			m_BitField |= m_BitField + 1;

			return emptyIndex;
		}

		bool RemoveObserver(ListenerIndex index)
		{
			// Check for invalid index
			KG_ASSERT(index < m_Listeners.size());

			size_t indexMask{ ((ListenerIndex)1 << index) };

			// Ensure bit is already set
			if (!(m_BitField & indexMask))
			{
				// Bit was not set
				return false;
			}

			// Clear the functor
			m_Listeners[index] = nullptr;

			// Set the indicated bit index to 0
			m_BitField &= ~indexMask;

			return true;
		}

		void Clear()
		{
			m_BitField = 0;
			m_Listeners.clear();
		}
	
	private:
		//==============================
		// Internal Fields
		//==============================
		std::vector<std::function<void(Args... args)>> m_Listeners{};
		size_t m_BitField{0};
	};
}

