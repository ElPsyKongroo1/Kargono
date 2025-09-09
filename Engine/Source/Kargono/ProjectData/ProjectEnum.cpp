#include "kgpch.h"
#include "ProjectEnum.h"

namespace Kargono::ProjectData
{
	bool ProjectEnum::DoesContainIdentifier(const char* queryName)
	{
		KG_ASSERT(queryName);

		for (const FixedBufStr32& currentIdentifier : m_EnumIdentifiers)
		{
			if (strcmp(currentIdentifier.CString(), queryName) == 0) // TODO: Fix this
			{
				return true;
			}
		}
		return false;
	}
	bool ProjectEnum::RemoveIdentifier(const char* queryName)
	{
		constexpr size_t k_InvalidIdentifierIndex{ std::numeric_limits<size_t>().max() };

		KG_ASSERT(queryName);
		size_t indexToDelete{ k_InvalidIdentifierIndex };
		size_t iteration{ 0 };

		// Search through identifiers for matching name
		for (const FixedBufStr32& currentIdentifier : m_EnumIdentifiers)
		{
			if (strcmp(currentIdentifier.CString(), queryName) == 0) // TODO: Fix this
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
		KG_ASSERT(indexToDelete < m_EnumIdentifiers.size());

		// Remove the identifier
		m_EnumIdentifiers.erase(m_EnumIdentifiers.begin() + indexToDelete);

		return true;
	}
	bool ProjectEnum::RemoveIdentifier(size_t indexToDelete)
	{
		constexpr size_t k_InvalidIdentifierIndex{ std::numeric_limits<size_t>().max() };

		// Ensure provided index is valid
		KG_ASSERT(indexToDelete != k_InvalidIdentifierIndex);
		KG_ASSERT(indexToDelete < m_EnumIdentifiers.size());


		// Remove the identifier
		m_EnumIdentifiers.erase(m_EnumIdentifiers.begin() + indexToDelete);

		return true;
	}
	bool ProjectEnum::RenameIdentifier(size_t indexToModify, const char* newName)
	{
		constexpr size_t k_InvalidIdentifierIndex{ std::numeric_limits<size_t>().max() };

		// Ensure the argument values are valid
		KG_ASSERT(newName);
		KG_ASSERT(indexToModify != k_InvalidIdentifierIndex);
		KG_ASSERT(indexToModify < m_EnumIdentifiers.size());
		size_t iteration{ 0 };

		// Search through identifiers and ensure no duplicate name is found
		for (const FixedBufStr32& currentIdentifier : m_EnumIdentifiers)
		{
			if (strcmp(currentIdentifier.CString(), newName) == 0)
			{
				KG_WARN("Failed to rename an identifier. Duplicate name found!");
				return false;
			}
			iteration++;
		}

		// Get the indicated identifier and modify its name
		FixedBufStr32& indicatedIdentifier = m_EnumIdentifiers.at(indexToModify);
		indicatedIdentifier = newName;

		return true;
	}
}
