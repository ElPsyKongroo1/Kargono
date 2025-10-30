#pragma once

#include "Kargono/Core/FixedBufferString.h"
#include "Modules/ECSInternal/Module/ComponentTag.h"
#include "Modules/Core/Module/CoreModule.h"

#include <string>

namespace Kargono
{
	struct Tag
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		Tag() = default;
		Tag(std::string_view tag) : m_Tag(tag) {}
		Tag(std::string_view tag, std::string_view group) : m_Tag(tag), m_Group(group) {}

	public:
		//==============================
		// Copy Function(s)
		//==============================
		void CopyTo(Tag* dst)
		{
			// Create the component in place
			std::construct_at<Tag>(dst);

			dst->m_Tag = m_Tag;
			dst->m_Group = m_Group;
		}
	public:
		//==============================
		// Serialization
		//==============================
		void Serialize(void* context);
		void Deserialize(void* context);
	public:
		//==============================
		// Public Fields
		//==============================
		FixedBufStr32 m_Tag{};
		FixedBufStr32 m_Group{};
	};

	Register_Module_Type(Tag, ECSInternal::ComponentTag)
}