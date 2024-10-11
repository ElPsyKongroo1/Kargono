#pragma once

#include "Kargono/Core/Base.h"

#include <cstdint>
#include <type_traits>
#include <limits>

namespace Kargono
{
	//=========================
	// Bitfield Class
	//=========================
	template <typename DataType>
	class BitField 
	{
		static_assert(std::is_integral<DataType>::value, "BitField only supports integral types.");
		static_assert(sizeof(DataType) == 1 || sizeof(DataType) == 2 || sizeof(DataType) == 4 || sizeof(DataType) == 8,
			"BitField supports 1, 2, 4, and 8 byte integral types.");

	public:
		//=========================
		// Constructor/Destructor
		//=========================
		BitField() = default;

		//=========================
		// Modify Specific Flags
		//=========================
		void SetFlag(uint8_t flag) 
		{
			KG_ASSERT(flag < sizeof(DataType) * 8);
			m_Bitfield |= (1 << flag);
		}
		void ClearFlag(uint8_t flag) 
		{
			KG_ASSERT(flag < sizeof(DataType) * 8);
			
			m_Bitfield &= ~(1 << flag);
			
		}
		void ToggleFlag(uint8_t flag) 
		{
			KG_ASSERT(flag < sizeof(DataType) * 8);
			m_Bitfield ^= (1 << flag);
		}

		//=========================
		// Modify All Flags
		//=========================

		void ClearAllFlags()
		{
			m_Bitfield = 0;
		}

		void EnableAllFlags()
		{
			m_Bitfield = std::numeric_limits<DataType>::max();
		}

		//=========================
		// Query Flags
		//=========================
		bool IsFlagSet(uint8_t flag) const 
		{
			KG_ASSERT(flag < sizeof(DataType) * 8);

			return m_Bitfield & (1 << flag);
		}

		//=========================
		// Getters/Setters Core Data
		//=========================
		DataType GetRawBitfield() const 
		{
			return m_Bitfield;
		}

		// Set the bitfield directly (for bulk operations)
		void SetRawBitfield(DataType rawValue) 
		{
			m_Bitfield = rawValue;
		}

		//=========================
		// Operator Overloads
		//=========================
		operator bool() const
		{
			return (bool)m_Bitfield;
		}

	private:
		//=========================
		// Core Data
		//=========================
		DataType m_Bitfield{0};
	};
}
