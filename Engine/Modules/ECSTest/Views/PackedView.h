#pragma once

#include "Modules/ECSTest/Views/IViewTest.h"
#include "Modules/ECSTest/DataStructures/SparseSetTest.h"

#include "Kargono/Core/Base.h"

#include <span>
#include <tuple>
#include <type_traits>
#include <utility>
#include <cstdint>

namespace Kargono::ECS
{
    using PackedSparseSet = SparseSet<EntityID, ComponentIndex>;

    template<size_t t_NumComponents>
    using PackedStorage_t = std::conditional_t<
        t_NumComponents == 1,
        std::span<EntityID>,
        std::array<PackedSparseSet*, t_NumComponents>>;

    template<size_t t_NumComponents>
    class PackedIterator
    {
    public:
        //==============================
        // Metaprogramming Types & Asserts
        //==============================
        static_assert(t_NumComponents != 0);
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
        explicit PackedIterator(PackedStorage_t<t_NumComponents> elements, size_t index)
            : m_EntityStorage{ elements }, m_CurrentIndex(index) 
        {
            if constexpr (t_NumComponents > 1)
            {
                SkipToNextValid();
            }
        }
    public:
        //==============================
        // Retrieve Data
        //==============================
        EntityID& operator*() const
        {
            if constexpr (t_NumComponents == 1)
            {
                KG_ASSERT(m_CurrentIndex < m_EntityStorage.size());
                return m_EntityStorage[m_CurrentIndex];
            }
            else
            {
                KG_ASSERT(m_CurrentIndex < m_EntityStorage[0]->GetDenseCount());
                // TODO: This seems messy (creating a span every time!!!)
                return m_EntityStorage[0]->GetDenseList()[m_CurrentIndex];
            }
        }

        //==============================
        // Advance Iterator
        //==============================
        PackedIterator& operator++()
        {
            if constexpr (t_NumComponents == 1)
            {
                ++m_CurrentIndex;
                return *this;
            }
            else
            {
                ++m_CurrentIndex;
                SkipToNextValid();
                return *this;
            }
        }

        PackedIterator operator++(int)
        {    
            PackedIterator previousIter = *this;
            ++*this;
            return previousIter;
        }

    private:
        // Helper(s)
        void SkipToNextValid()
        {
            while (m_CurrentIndex < m_EntityStorage[0]->GetDenseCount() &&
                !AreAllComponentsValid(m_EntityStorage[0]->GetDenseList()[m_CurrentIndex]))
            {
                ++m_CurrentIndex;
            }
        }

        // Comparison helper(s)
        template<std::size_t... t_IndexSeq>
        bool AreAllComponentsValidImpl(EntityID currentID, std::index_sequence<t_IndexSeq...>) const
        {
            // Fold over && to ensure all spans have equal .data()
            return (m_EntityStorage[t_IndexSeq]->HasSparseIndex(currentID) && ...);
        }

        bool AreAllComponentsValid(EntityID currentID) const
        {
            // Create index sequence to iterate through all spans (0 to t_NumComponents - 1)
            // TODO: Optimization (1 to t_NumComponents - 1) since index 0 would already be validated
            return AreAllComponentsValidImpl(currentID, std::make_index_sequence<t_NumComponents>{});
        }

    public:
        //==============================
        // Comparison Operator(s)
        //==============================
        bool operator==(const PackedIterator& other) const
        {
            if constexpr (t_NumComponents == 1)
            {
                return m_EntityStorage.data() == other.m_EntityStorage.data() &&
                    m_CurrentIndex == other.m_CurrentIndex;
            }
            else
            {
                // Check all spans for equality and validate m_CurrentIndex as usual
                return SpanDataEqual(other) && m_CurrentIndex == other.m_CurrentIndex;
            }
        }

    private:
        // Comparison helper(s)
        template<std::size_t... t_IndexPack>
        bool SpanDataEqualImpl(const PackedIterator& other, std::index_sequence<t_IndexPack...>) const
        {
            // Fold over && to ensure all spans have equal .data()
            return ((m_EntityStorage[t_IndexPack] == other.m_EntityStorage[t_IndexPack]) && ...);
        }

        bool SpanDataEqual(const PackedIterator& other) const
        {
            // Create index sequence to iterate through all spans (0 to t_NumComponents - 1)
            return SpanDataEqualImpl(other, std::make_index_sequence<t_NumComponents>{});
        }
    private:
        //==============================
        // Internal Fields
        //==============================
        PackedStorage_t<t_NumComponents> m_EntityStorage{};
        size_t m_CurrentIndex{ 0 };
    };

    template<typename... t_ComponentTypes>
	class PackedView : public IView
	{
    public:
        //==============================
        // Metaprogramming Types & Asserts
        //==============================
        static constexpr size_t k_NumComponents{ sizeof...(t_ComponentTypes) };
        static_assert(k_NumComponents != 0);
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		PackedView() = default;
		PackedView(PackedStorage_t<k_NumComponents> entityList) :
			m_EntityStorage(entityList) {};
		~PackedView() = default;
	public:
		//==============================
		// Enable For-Loop / Iterator Usage
		//==============================
		PackedIterator<k_NumComponents> begin() const
		{
		    return PackedIterator<k_NumComponents>(m_EntityStorage, 0 );
		};
		PackedIterator<k_NumComponents> end() const
		{
            if constexpr (k_NumComponents == 1)
            {
			    return PackedIterator<k_NumComponents>( m_EntityStorage, m_EntityStorage.size() );
            }
            else
            {
                // TODO: Note this iterator uses the first component list as the basis for iteration
                // Might want to use the shortest list (determined at runtime) for faster results
                return PackedIterator<k_NumComponents>(m_EntityStorage, m_EntityStorage[0]->GetDenseCount());
            }
		};
	private:
		//==============================
		// Internal Fields
		//==============================
        PackedStorage_t<k_NumComponents> m_EntityStorage;
	};
}