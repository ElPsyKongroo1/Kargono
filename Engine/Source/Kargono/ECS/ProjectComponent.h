#pragma once

#include "Kargono/Core/Base.h"
#include "Kargono/Core/WrappedData.h"

#include <array>
#include <cstdint>
#include <vector>
#include <string>
#include <limits>
#include <algorithm>


namespace Kargono::ECS
{
	inline static std::array<uint64_t, 32> s_AllBufferSizes 
	{
		4, 8, 12, 16, 20, 24, 28, 32,
		40, 48, 56, 64, 72, 80, 88, 96,
		112, 128, 144, 160, 176, 192, 208, 224,
		256, 288, 320, 352, 384, 416, 448, 480
	};

	inline uint64_t GetBufferSizeFromComponentSize(uint64_t componentSize)
	{
		auto upperBoundRef = std::lower_bound(s_AllBufferSizes.begin(), s_AllBufferSizes.end(), componentSize);
		KG_ASSERT(upperBoundRef != s_AllBufferSizes.end(), "Could not locate buffer value that contains component size properly");
		return *upperBoundRef;
	}

	constexpr uint16_t InvalidBufferSlot = std::numeric_limits<int16_t>::max();

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
		static bool AddFieldToProjectComponent(Ref<ProjectComponent> component, WrappedVarType fieldType, const std::string& fieldName)
		{
			KG_ASSERT(component, "Invalid component provided when attempting to add a field to project component");
			KG_ASSERT(component->m_DataNames.size() == component->m_DataLocations.size() &&
				component->m_DataLocations.size() == component->m_DataTypes.size(), "Project component is malformed");

			// Ensure fieldType is valid
			if (fieldType == WrappedVarType::None)
			{
				KG_WARN("Could not add field to project component. Cannot instantiate a project component field with \"None\" type");
				return false;
			}

			// Ensure duplicate name is not provided
			bool foundDuplicateName{ false };
			for (const std::string& name : component->m_DataNames)
			{
				if (fieldName == name)
				{
					foundDuplicateName = true;
					break;
				}
			}
			if (foundDuplicateName)
			{
				KG_WARN("Could not add field to project component. Duplicate field name provided");
				return false;
			}

			// Add Field Location inside buffer and update new maximum size of buffer
			size_t currentMaximumLocation{ 0 };
			for (WrappedVarType type : component->m_DataTypes)
			{
				currentMaximumLocation += Utility::WrappedVarTypeToDataSizeBytes(type);
			}
			component->m_DataLocations.push_back(currentMaximumLocation);
			currentMaximumLocation += Utility::WrappedVarTypeToDataSizeBytes(fieldType);
			component->m_ComponentSize = currentMaximumLocation;
			component->m_BufferSize = GetBufferSizeFromComponentSize(component->m_ComponentSize);

			// Store Field Name and Field Type
			component->m_DataNames.push_back(fieldName);
			component->m_DataTypes.push_back(fieldType);
			return true;
		}

		static void DeleteFieldFromProjectComponent(Ref<ProjectComponent> component,size_t fieldIndex)
		{
			KG_ASSERT(component, "Invalid component provided when attempting to delete field from project component");
			KG_ASSERT(fieldIndex < component->m_DataNames.size(), "Out-of-bounds index provided to delete field from project component function");

			KG_ASSERT(component->m_DataNames.size() == component->m_DataLocations.size() &&
				component->m_DataLocations.size() == component->m_DataTypes.size(), "Project component is malformed");

			// Delete index from all vectors
			component->m_DataNames.erase(component->m_DataNames.begin() + fieldIndex);
			component->m_DataTypes.erase(component->m_DataTypes.begin() + fieldIndex);
			component->m_DataLocations.erase(component->m_DataLocations.begin() + fieldIndex);

			// Recalculate data locations
			size_t iteration = 0;
			size_t currentDataLocation{ 0 };
			for (WrappedVarType type : component->m_DataTypes)
			{
				component->m_DataLocations.at(iteration) = currentDataLocation;
				currentDataLocation += Utility::WrappedVarTypeToDataSizeBytes(type);
				iteration++;
			}

			// Store new buffer size
			component->m_ComponentSize = currentDataLocation;
			component->m_BufferSize = GetBufferSizeFromComponentSize(component->m_ComponentSize);
		}

		static bool EditFieldInProjectComponent(Ref<ProjectComponent> component, size_t fieldIndex, const std::string& fieldName, WrappedVarType fieldType)
		{
			KG_ASSERT(component, "Invalid component provided when attempting to edit field in project component");
			KG_ASSERT(component->m_DataNames.size() == component->m_DataLocations.size() &&
				component->m_DataLocations.size() == component->m_DataTypes.size(), "Project component is malformed");

			// Ensure fieldType is valid
			if (fieldType == WrappedVarType::None)
			{
				KG_WARN("Could not edit project component. Cannot instantiate a project component field with \"None\" type");
				return false;
			}

			// Ensure duplicate name is not provided
			bool foundDuplicateName{ false };
			for (const std::string& name : component->m_DataNames)
			{
				if (fieldName == name)
				{
					foundDuplicateName = true;
					break;
				}
			}
			if (foundDuplicateName)
			{
				KG_WARN("Could not edit project component. Duplicate field name provided");
				return false;
			}

			// Edit Field
			component->m_DataNames.at(fieldIndex) = fieldName;
			component->m_DataTypes.at(fieldIndex) = fieldType;


			// Recalculate data locations
			size_t iteration = 0;
			size_t currentDataLocation{ 0 };
			for (WrappedVarType type : component->m_DataTypes)
			{
				component->m_DataLocations.at(iteration) = currentDataLocation;
				currentDataLocation += Utility::WrappedVarTypeToDataSizeBytes(type);
				iteration++;
			}

			// Store new buffer size
			component->m_ComponentSize = currentDataLocation;
			component->m_BufferSize = GetBufferSizeFromComponentSize(component->m_ComponentSize);
			return true;
		}
	};
}
