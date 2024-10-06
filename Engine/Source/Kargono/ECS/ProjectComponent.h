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
		auto upperBoundRef = std::upper_bound(s_AllBufferSizes.begin(), s_AllBufferSizes.end(), componentSize);
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
		static void AddFieldToProjectComponent(Ref<ProjectComponent> component, WrappedVarType fieldType, const std::string& fieldName = {})
		{
			// Generate a name for the field, if one is not presented
			std::string fieldNameCache {fieldName};
			if (fieldName.empty())
			{
				// Generate a name
				size_t iteration{ 1 };
				while (true)
				{
					bool foundCopy = false;
					for (std::string& componentName : component->m_DataNames)
					{
						if (componentName == "New Field " + std::to_string(iteration))
						{
							foundCopy = true;
							break;
						}
					}
					if (!foundCopy)
					{
						fieldNameCache = "New Field " + std::to_string(iteration);
						break;
					}
					iteration++;
				}
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
			component->m_DataNames.push_back(fieldNameCache);
			component->m_DataTypes.push_back(fieldType);
		}
	};
}
