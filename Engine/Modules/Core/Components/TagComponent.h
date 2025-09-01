#pragma once

#include "Kargono/Core/FixedBufferString.h"
#include "Modules/ECSInternal/Module/ComponentTag.h"
#include "Modules/Core/Module/CoreModule.h"

#include <string>

namespace Kargono
{
	struct TagComponent
	{
		//==============================
		// Constructors/Destructors
		//==============================
		TagComponent() = default;
		TagComponent(std::string_view tag) : m_Tag(tag) {}
		TagComponent(std::string_view tag, std::string_view group) : m_Tag(tag), m_Group(group) {}

		//==============================
		// Copy Function(s)
		//==============================
		void CopyTo(TagComponent* dst)
		{
			TagComponent* destination = (TagComponent*)dst;
			destination->m_Tag = m_Tag;
			destination->m_Group = m_Group;
		}

		//==============================
		// Public Fields
		//==============================
		FixedBufStr32 m_Tag{};
		FixedBufStr32 m_Group{};
	};

	Register_Module_Type(TagComponent, ECSInternal::ComponentTag)
}