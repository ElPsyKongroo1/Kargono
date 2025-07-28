#pragma once

#include "Modules/ECSTest/CommonTest.h"

#include <iterator>
#include <vector> // TODO: Maybe delete this?
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
        explicit PackedIterator(std::span<EntityID> elements, size_t index )
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


	class IView
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		IView() = default;
		virtual ~IView() = default;
    public:
        //==============================
        // Enable For-Loop / Iterator Usage
        //==============================
        virtual PackedIterator begin() const = 0;
        virtual PackedIterator end() const = 0;
	};
}