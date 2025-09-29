#pragma once

#include "Kargono/Core/FixedBufferString.h"

#include <vector>

namespace Kargono::Scripting
{
	struct CustomEnum
	{
	public:
		//=========================
		// Static 
		//=========================
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
		FixedBufStr32 m_EnumName{};
		std::vector<FixedBufStr32> m_EnumIdentifiers{};
	};
}
