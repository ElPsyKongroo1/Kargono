#pragma once
#include "Kargono/Core/FixedString.h"
#include "Kargono/Core/WrappedData.h"
#include "Kargono/Core/Buffer.h"

#include <cstdint>
#include <vector>

namespace Kargono::ProjectData
{
	struct GlobalState
	{
		FixedString32 m_Name;
		std::vector<WrappedVarType> m_DataTypes;
		std::vector<size_t> m_DataLocations;
		std::vector<FixedString32> m_DataNames;
		size_t m_BufferSize{ 0 };
		Buffer m_DataBuffer;
	};

	class GlobalStateService
	{
	public:

	};
}
