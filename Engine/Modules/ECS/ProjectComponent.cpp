#include "kgpch.h"

#include "Modules/ECS/ProjectComponent.h"

namespace Kargono::ECS
{
	bool ProjectComponent::AddField(WrappedVarType fieldType, const char* fieldName)
	{
		KG_ASSERT(m_DataNames.size() == m_DataLocations.size() &&
			m_DataLocations.size() == m_DataTypes.size(), "Project component is malformed");

		// Ensure fieldType is valid
		if (fieldType == WrappedVarType::None || fieldType == WrappedVarType::Void)
		{
			KG_WARN("Could not add field to project component. Cannot instantiate a project component field with \"None\" type");
			return false;
		}

		// Ensure duplicate name is not provided
		bool foundDuplicateName{ false };
		for (const char* name : m_DataNames)
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
		for (WrappedVarType type : m_DataTypes)
		{
			currentMaximumLocation += Utility::WrappedVarTypeToDataSizeBytes(type);
		}
		m_DataLocations.push_back(currentMaximumLocation);
		currentMaximumLocation += Utility::WrappedVarTypeToDataSizeBytes(fieldType);
		m_ComponentSize = currentMaximumLocation;

		// Store Field Name and Field Type
		m_DataNames.push_back(fieldName);
		m_DataTypes.push_back(fieldType);
		return true;
	}
	void ProjectComponent::DeleteField(size_t fieldIndex)
	{
		KG_ASSERT(fieldIndex < m_DataNames.size());

		KG_ASSERT(m_DataNames.size() == m_DataLocations.size() &&
			m_DataLocations.size() == m_DataTypes.size(), "Project component is malformed");

		// Delete index from all vectors
		m_DataNames.erase(m_DataNames.begin() + fieldIndex);
		m_DataTypes.erase(m_DataTypes.begin() + fieldIndex);
		m_DataLocations.erase(m_DataLocations.begin() + fieldIndex);

		// Recalculate data locations
		size_t iteration = 0;
		size_t currentDataLocation{ 0 };
		for (WrappedVarType type : m_DataTypes)
		{
			m_DataLocations.at(iteration) = currentDataLocation;
			currentDataLocation += Utility::WrappedVarTypeToDataSizeBytes(type);
			iteration++;
		}

		// Store new buffer size
		m_ComponentSize = currentDataLocation;
	}
	bool ProjectComponent::EditField(size_t fieldIndex, const char* fieldName, WrappedVarType fieldType)
	{
		KG_ASSERT(m_DataNames.size() == m_DataLocations.size() &&
			m_DataLocations.size() == m_DataTypes.size(), "Project component is malformed");

		// Ensure fieldType is valid
		if (fieldType == WrappedVarType::None || fieldType == WrappedVarType::Void)
		{
			KG_WARN("Could not edit project component. Cannot instantiate a project component field with \"None\" type");
			return false;
		}

		// Edit Field
		m_DataNames.at(fieldIndex) = fieldName;
		m_DataTypes.at(fieldIndex) = fieldType;

		// Recalculate data locations
		size_t iteration = 0;
		size_t currentDataLocation{ 0 };
		for (WrappedVarType type : m_DataTypes)
		{
			m_DataLocations.at(iteration) = currentDataLocation;
			currentDataLocation += Utility::WrappedVarTypeToDataSizeBytes(type);
			iteration++;
		}

		// Store new buffer size
		m_ComponentSize = currentDataLocation;
		return true;
	}
}
