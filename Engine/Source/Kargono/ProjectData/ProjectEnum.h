#pragma once

#include "Kargono/Core/FixedString.h"

#include <vector>

namespace Kargono::ProjectData
{
	struct ProjectEnum
	{
	public:
		//=========================
		// Query State
		//=========================
		bool DoesContainIdentifier(const char* queryName);
	public:
		//=========================
		// Modify State
		//=========================
		bool RemoveIdentifier(const char* queryName);
		bool RemoveIdentifier(size_t indexToDelete);
		bool RenameIdentifier(size_t indexToModify, const char* newName);
	public:
		//=========================
		// Public Fields
		//=========================
		FixedString32 m_EnumName;
		std::vector<FixedString32> m_EnumIdentifiers;
	};
}
