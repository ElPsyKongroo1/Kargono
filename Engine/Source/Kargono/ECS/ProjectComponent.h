#pragma once

#include "Kargono/Core/WrappedData.h"

#include <cstdint>
#include <vector>
#include <string>
#include <limits>


namespace Kargono::ECS
{
	constexpr uint16_t InvalidBufferSlot = std::numeric_limits<int16_t>::max();

	struct ProjectComponent
	{
		std::string m_Name;
		uint32_t m_ComponentSize{ 0 };
		uint32_t m_BufferSize{ 0 };
		uint16_t m_BufferSlot { InvalidBufferSlot };
		std::vector<WrappedVarType> m_DataTypes;
		std::vector<uint32_t> m_DataLocations;
		std::vector<std::string> m_DataNames;
	};
}
