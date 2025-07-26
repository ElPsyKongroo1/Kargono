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
	public:
		//=========================
		// Constructor/Destructor
		//=========================
		BitField() = default;
		BitField(t_DataType defaultValue) : m_Bitfield(defaultValue) {}

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

		void ClearAllFlags()
		{
			m_Bitfield = 0;
		}

		void EnableAllFlags()
		{
			m_Bitfield = std::numeric_limits<t_DataType>::max();
		}

		//=========================
		// Query Flags
		//=========================
		bool IsFlagSet(uint8_t flag) const
		{
			KG_ASSERT(flag < sizeof(t_DataType) * 8);

			return m_Bitfield & (1 << flag);
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
