#include "kgpch.h"
#include "ProjectEnum.h"

namespace Kargono::ProjectData
{
	bool Kargono::ProjectData::ProjectEnumService::DoesProjectEnumContainIdentifier(ProjectEnum* enumToCheck, const char* queryName)
	{
		KG_ASSERT(enumToCheck);
		KG_ASSERT(queryName);

		for (const FixedString32& currentIdentifier : enumToCheck->m_EnumIdentifiers)
		{
			if (strcmp(currentIdentifier.CString(), queryName) == 0)
			{
				return true;
			}
		}
		return false;
	}
	bool ProjectEnumService::RemoveIdentifier(ProjectEnum* enumToModify, const char* queryName)
	{
		constexpr size_t k_InvalidIdentifierIndex{ std::numeric_limits<size_t>().max() };

		KG_ASSERT(enumToModify);
		KG_ASSERT(queryName);
		size_t indexToDelete{ k_InvalidIdentifierIndex };
		size_t iteration{ 0 };

		// Search through identifiers for matching name
		for (const FixedString32& currentIdentifier : enumToModify->m_EnumIdentifiers)
		{
			if (strcmp(currentIdentifier.CString(), queryName) == 0)
			{
				indexToDelete = iteration;
				break;
			}
			iteration++;
		}


		// Check if no index was found
		if (indexToDelete == k_InvalidIdentifierIndex)
		{
			KG_WARN("Failed to remove identifier from the indicated enum");
			return false;
		}

		// Ensure found index is not out of bounds
		KG_ASSERT(indexToDelete < enumToModify->m_EnumIdentifiers.size());

		// Remove the identifier
		enumToModify->m_EnumIdentifiers.erase(enumToModify->m_EnumIdentifiers.begin() + indexToDelete);

		return true;
	}
	bool ProjectEnumService::RemoveIdentifier(ProjectEnum* enumToModify, size_t indexToDelete)
	{
		constexpr size_t k_InvalidIdentifierIndex{ std::numeric_limits<size_t>().max() };

		// Ensure provided index is valid
		KG_ASSERT(enumToModify);
		KG_ASSERT(indexToDelete != k_InvalidIdentifierIndex);
		KG_ASSERT(indexToDelete < enumToModify->m_EnumIdentifiers.size());


		// Remove the identifier
		enumToModify->m_EnumIdentifiers.erase(enumToModify->m_EnumIdentifiers.begin() + indexToDelete);

		return true;
	}
	bool ProjectEnumService::RenameIdentifier(ProjectEnum* enumToModify, size_t indexToModify, const char* newName)
	{
		constexpr size_t k_InvalidIdentifierIndex{ std::numeric_limits<size_t>().max() };

		// Ensure the argument values are valid
		KG_ASSERT(enumToModify);
		KG_ASSERT(newName);
		KG_ASSERT(indexToModify != k_InvalidIdentifierIndex);
		KG_ASSERT(indexToModify < enumToModify->m_EnumIdentifiers.size());
		size_t iteration{ 0 };

		// Search through identifiers and ensure no duplicate name is found
		for (const FixedString32& currentIdentifier : enumToModify->m_EnumIdentifiers)
		{
			if (strcmp(currentIdentifier.CString(), newName) == 0)
			{
				KG_WARN("Failed to rename an identifier. Duplicate name found!");
				return false;
			}
			iteration++;
		}

		// Get the indicated identifier and modify its name
		FixedString32& indicatedIdentifier = enumToModify->m_EnumIdentifiers.at(indexToModify);
		indicatedIdentifier = newName;

		return true;
	}
}
