#pragma once

#include "Kargono/Core/FixedString.h"

#include <vector>

namespace Kargono::ProjectData
{
	struct ProjectEnum
	{
		FixedString32 m_EnumName;
		std::vector<FixedString32> m_EnumIdentifiers;
	};

	class ProjectEnumService
	{
	public:
		static bool DoesProjectEnumContainIdentifier(ProjectEnum* enumToCheck, const char* queryName);
		static bool RemoveIdentifier(ProjectEnum* enumToModify, const char* queryName);
		static bool RemoveIdentifier(ProjectEnum* enumToModify, size_t indexToDelete);
		static bool RenameIdentifier(ProjectEnum* enumToModify, size_t indexToModify, const char* newName);
	};
}
