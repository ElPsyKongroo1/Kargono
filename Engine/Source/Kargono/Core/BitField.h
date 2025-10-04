#pragma once

#include "Kargono/Core/Base.h"

#include <cstdint>
#include <concepts>
#include <limits>

namespace Kargono
{
	template <std::unsigned_integral t_DataType = size_t>
	class BitField
	{
		static_assert(sizeof(t_DataType) == 1 || sizeof(t_DataType) == 2 || sizeof(t_DataType) == 4 || sizeof(t_DataType) == 8,
			"BitField supports 1, 2, 4, and 8 byte integral types.");
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		constexpr BitField() = default;
		constexpr BitField(t_DataType defaultValue) : m_Bitfield(defaultValue) {}

		//=========================
		// Modify Specific Flags
		//=========================
		void SetFlag(uint8_t flag)
		{
			KG_ASSERT(flag < sizeof(t_DataType) * 8);
			m_Bitfield |= (1 << flag);
		}
		void ClearFlag(uint8_t flag)
		{
			KG_ASSERT(flag < sizeof(t_DataType) * 8);
			m_Bitfield &= ~(1 << flag);
		}
		void ToggleFlag(uint8_t flag)
		{
			KG_ASSERT(flag < sizeof(t_DataType) * 8);
			m_Bitfield ^= (1 << flag);
		}

		//=========================
		// Modify All Flags
		//=========================
		constexpr void ClearAllFlags()
		{
			m_Bitfield = 0;
		}

		constexpr void EnableAllFlags()
		{
			m_Bitfield = std::numeric_limits<t_DataType>::max();
		}

		//=========================
		// Query Flags
		//=========================
		constexpr bool IsFlagSet(uint8_t flag) const
		{
			KG_ASSERT(flag < sizeof(t_DataType) * 8);

			return m_Bitfield & (1 << flag);
		}

		bool IsSingleBitSet() 
		{
			return m_Bitfield != 0 && (m_Bitfield & (m_Bitfield - 1)) == 0;
		}

		//=========================
		// Getters/Setters Core Data
		//=========================
		t_DataType GetRawBitfield() const
		{
			return m_Bitfield;
		}

		// Basically the assignment operator
		void SetRawBitfield(t_DataType rawValue)
		{
			m_Bitfield = rawValue;
		}

		//=========================
		// Operator Overloads
		//=========================
		operator t_DataType() const
		{
			return m_Bitfield;
		}
		
		operator bool() const
		{
			return (bool)m_Bitfield;
		}

	public:
		//=========================
		// Core Data
		//=========================
		t_DataType m_Bitfield{ 0 };
	};
}
