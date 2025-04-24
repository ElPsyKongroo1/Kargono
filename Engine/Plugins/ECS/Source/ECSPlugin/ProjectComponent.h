#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Core/WrappedData.h"
#include "ECSPlugin/ECSCommon.h"

#include <array>
#include <cstdint>
#include <vector>
#include <string>
#include <limits>
#include <algorithm>


namespace Kargono::ECS
{
	struct ProjectComponent
	{
		std::string m_Name;
		uint64_t m_ComponentSize{ 0 };
		uint64_t m_BufferSize{ 0 };
		uint16_t m_BufferSlot { InvalidBufferSlot };
		std::vector<WrappedVarType> m_DataTypes;
		std::vector<uint64_t> m_DataLocations;
		std::vector<std::string> m_DataNames;
	};

	class ProjectComponentService
	{
	public:
		static bool AddFieldToProjectComponent(Ref<ProjectComponent> component, WrappedVarType fieldType, const std::string& fieldName);
		static void DeleteFieldFromProjectComponent(Ref<ProjectComponent> component, size_t fieldIndex);
		static bool EditFieldInProjectComponent(Ref<ProjectComponent> component, size_t fieldIndex, const std::string& fieldName, WrappedVarType fieldType);
	};
}
