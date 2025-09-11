#include "kgpch.h"

#include "Modules/ECSInternal/CustomComponent.h"

namespace Kargono::ECSInternal
{
	bool CustomComponent::AddField(WrappedVarType fieldType, const char* fieldName)
	{
		KG_ASSERT(m_DataNames.size() == m_DataOffsets.size() &&
			m_DataOffsets.size() == m_DataTypes.size(), "Project component is malformed");

		// Ensure fieldType is valid
		if (fieldType == WrappedVarType::None || fieldType == WrappedVarType::Void)
		{
			KG_WARN("Could not add field to custom component. Cannot instantiate a custom component field with \"None\" type");
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
			KG_WARN("Could not add field to custom component. Duplicate field name provided");
			return false;
		}

		// Store Field Name and Field Type
		m_DataNames.push_back(fieldName);
		m_DataTypes.push_back(fieldType);

		// Recalculate alignment
		size_t alignment = RevalidateAlignment();

		// Recalculate data locations
		RecalculateDataLocations();

		return true;
	}
	void CustomComponent::DeleteField(size_t fieldIndex)
	{
		KG_ERROR("Make sure to take into account alignment");

		KG_ASSERT(fieldIndex < m_DataNames.size());

		KG_ASSERT(m_DataNames.size() == m_DataOffsets.size() &&
			m_DataOffsets.size() == m_DataTypes.size(), "Project component is malformed");

		// Delete index from all vectors
		m_DataNames.erase(m_DataNames.begin() + fieldIndex);
		m_DataTypes.erase(m_DataTypes.begin() + fieldIndex);
		m_DataOffsets.erase(m_DataOffsets.begin() + fieldIndex);

		// Revalidate alignment and data locations
		RevalidateAlignment();
		RecalculateDataLocations();
	}
	bool CustomComponent::EditField(size_t fieldIndex, const char* fieldName, WrappedVarType fieldType)
	{
		KG_ERROR("Make sure to take into account alignment");
		KG_ASSERT(m_DataNames.size() == m_DataOffsets.size() &&
			m_DataOffsets.size() == m_DataTypes.size(), "Project component is malformed");

		// Ensure fieldType is valid
		if (fieldType == WrappedVarType::None || fieldType == WrappedVarType::Void)
		{
			KG_WARN("Could not edit custom component. Cannot instantiate a custom component field with \"None\" type");
			return false;
		}

		// Edit Field
		m_DataNames.at(fieldIndex) = fieldName;
		m_DataTypes.at(fieldIndex) = fieldType;

		// Revalidate alignment and data locations
		RevalidateAlignment();
		RecalculateDataLocations();

		return true;
	}
	void CustomComponent::RecalculateDataLocations()
	{
		m_DataOffsets.clear();
		m_DataOffsets.reserve(m_DataTypes.size());
		size_t currentMaximumLocation{ 0 };
		for (WrappedVarType type : m_DataTypes)
		{
			size_t remainder = currentMaximumLocation % m_ComponentAlignment;
			if (remainder != 0)
			{
				currentMaximumLocation += m_ComponentAlignment - remainder;
			}
			m_DataOffsets.push_back(currentMaximumLocation);
			currentMaximumLocation += Utility::WrappedVarTypeToDataSizeBytes(type);
		}

		m_ComponentSize = currentMaximumLocation;
	}
	ECSInternal::ComponentIdentifier CustomComponent::RevalidateIdentifier()
	{
		KG_ASSERT(m_Name.StringLength() > 0);
		std::string identifierStr{ "CustomComponent" "::" + m_Name };
		ECSInternal::ComponentIdentifier identifier =
			Utility::FileSystem::CRCFromString(identifierStr.c_str());

		m_Identifier = identifier;

		return identifier;
	}
	ECSInternal::ComponentMetadata CustomComponent::GenerateMetadata(Assets::AssetHandle compHandle) const
	{
		KG_ASSERT(compHandle != Assets::k_EmptyHandle);

		ECSInternal::ComponentMetadata metadata{};
		metadata.m_ComponentSize = m_ComponentSize;
		metadata.m_ComponentAlignment = m_ComponentAlignment;
		metadata.m_CompFunctors.m_Copy = CustomComponentCopyTo;
		metadata.m_CustomComponentHandle = compHandle;
		return metadata;
	}
	size_t CustomComponent::RevalidateAlignment()
	{
		size_t maxAlignment{ 1 };
		for (WrappedVarType type : m_DataTypes)
		{
			size_t typeAlignment{ Utility::WrappedVarTypeAlignment(type) };
			if (typeAlignment > maxAlignment)
			{
				maxAlignment = typeAlignment;
			}
		}
		m_ComponentAlignment = maxAlignment;
		return maxAlignment;
	}
}
