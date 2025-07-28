#pragma once

#include "Modules/ECSTest/Views/IViewTest.h"

#include "Kargono/Core/Base.h"

#include <span>

namespace Kargono::ECS
{
    class PackedIterator
    {
    public:
        //==============================
        // Iterator Type Defs (enable C++ optimization)
        //==============================
        using value_type = EntityID;
        using difference_type = std::ptrdiff_t;
    public:
        //==============================
        // Constructors/Destructors
        //==============================
        explicit PackedIterator() = default;
        explicit PackedIterator(std::span<EntityID> elements, size_t index)
            : m_EntityList{ elements }, m_CurrentIndex(index) {}
    public:
        //==============================
        // Retrieve Data
        //==============================
        EntityID& operator*() const
        {
            KG_ASSERT(m_CurrentIndex < m_EntityList.size());
            return m_EntityList[m_CurrentIndex];
        }

        //==============================
        // Advance Iterator
        //==============================
        PackedIterator& operator++()
        {
            ++m_CurrentIndex;
            return *this;
        }

        PackedIterator operator++(int)
        {                       // 1
            PackedIterator previousIter = *this;
            ++*this;
            return previousIter;
        }

        //==============================
        // Comparison Operator(s)
        //==============================
        bool operator==(const PackedIterator& other) const
        {
            return m_EntityList.data() == other.m_EntityList.data() &&
                m_CurrentIndex == other.m_CurrentIndex;
        }

    private:
        //==============================
        // Internal Fields
        //==============================
        std::span<EntityID> m_EntityList{};
        std::size_t m_CurrentIndex{ 0 };
    };

	class PackedView : public IView
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		PackedView() = default;
		PackedView(std::span<EntityID> entityList) : 
			m_EntityListSpan(entityList) {};
		~PackedView() = default;
	public:
		//==============================
		// Enable For-Loop / Iterator Usage
		//==============================
		PackedIterator begin() const
		{
			return PackedIterator( m_EntityListSpan, 0 );
		};
		PackedIterator end() const
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