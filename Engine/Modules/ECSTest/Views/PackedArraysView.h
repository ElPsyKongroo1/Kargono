#pragma once

#include "Modules/ECSTest/Views/IViewTest.h"

#include "Kargono/Core/Base.h"

#include <span>

namespace Kargono::ECS
{
	class Registry;

	template<typename t_ComponentType>
	class PackedArraysView : public IView
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		PackedArraysView() = default;
		PackedArraysView(std::span<EntityID> entityList) : 
			m_EntityListSpan(entityList) {};
		~PackedArraysView() = default;
	public:
		//==============================
		// Enable For-Loop / Iterator Usage
		//==============================
		virtual PackedIterator begin() const
		{
			return PackedIterator( m_EntityListSpan, 0 );
		};
		virtual PackedIterator end() const
		{
			return PackedIterator( m_EntityListSpan, m_EntityListSpan.size() );
		};
	private:
		//==============================
		// Internal Fields
		//==============================
		std::span<EntityID> m_EntityListSpan;
	};
}