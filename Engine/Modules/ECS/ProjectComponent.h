#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Core/WrappedData.h"
#include "Modules/ECSInternal/ECSInternalCommon.h"

#include <array>
#include <cstdint>
#include <vector>
#include <string>
#include <limits>
#include <algorithm>


namespace Kargono::ECS
{
	using BufferSlot = uint16_t;
	constexpr uint16_t k_InvalidBufferSlot{ std::numeric_limits<BufferSlot>::max() };

	struct ProjectComponent
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		ProjectComponent() = default;
		~ProjectComponent() = default;
	public:
		//==============================
		// Interact w/ Fields
		//==============================
		bool AddField(WrappedVarType fieldType, const char* fieldName);
		void DeleteField(size_t fieldIndex);
		bool EditField(size_t fieldIndex, const char* fieldName, WrappedVarType fieldType);
	public:
		//==============================
		// Public Fields
		//==============================
		FixedBufStr32 m_Name;
		size_t m_ComponentSize{ 0 };
		size_t m_ComponentAlignment{ 0 };
		ECSInternal::ComponentIdentifier m_Identifier{ ECSInternal::k_InvalidComponentIdentifier };
		std::vector<WrappedVarType> m_DataTypes;
		std::vector<uint64_t> m_DataLocations;
		std::vector<FixedBufStr32> m_DataNames;
	};
}
