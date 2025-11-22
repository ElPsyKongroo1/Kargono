#pragma once

#include "Modules/ECSInternal/Module/ECSInternalModule.h"

#include <type_traits>
#include <span>
#include <array>

namespace Kargono::ECSInternal
{
    template<size_t t_NumComponents>
    using FlatStorage_t = std::conditional_t<
        t_NumComponents == 1,
        std::span<bool>,
        std::array<std::span<bool>, t_NumComponents>>;

    template<size_t t_NumComponents>
    class FlatIterator
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
        explicit FlatIterator() = default;
        explicit FlatIterator(FlatStorage_t<t_NumComponents> validEntitySpan, size_t index)
            : m_SpanStorage{ validEntitySpan }, m_CurrentEntity(static_cast<EntityID>(index)) 
        {
            SkipToNextValid();
        }
    public:
        //==============================
        // Retrieve Data
        //==============================
        EntityID operator*() const
        {
            if constexpr (t_NumComponents == 1)
            {
                KG_ASSERT(m_CurrentEntity < m_SpanStorage.size());
                return m_CurrentEntity;
            }
            else
            {
                KG_ASSERT(m_CurrentEntity < m_SpanStorage[0].size());
                return m_CurrentEntity;
            }
        }

    public:
        //==============================
        // Advance Iterator
        //==============================
        FlatIterator& operator++()
        {
            ++m_CurrentEntity;
            SkipToNextValid();
            return *this;
        }

        FlatIterator operator++(int)
        {                       // 1
            FlatIterator previousIter{ *this };
            ++*this;
            return previousIter;
        }
    private:
        // Helper(s)
        void SkipToNextValid()
        {
            if constexpr (t_NumComponents == 1)
            {
                while (m_CurrentEntity < m_SpanStorage.size() &&
                    !m_SpanStorage[m_CurrentEntity])
                {
                    ++m_CurrentEntity;
                }
            }
            else
            {
                while (m_CurrentEntity < m_SpanStorage[0].size() &&
                    !AreAllComponentsValid())
                {
                    ++m_CurrentEntity;
                }
            }
        }

        // Comparison helper(s)
        template<std::size_t... t_IndexSeq>
        bool AreAllComponentsValidImpl(std::index_sequence<t_IndexSeq...>) const
        {
            // Fold over && to ensure all spans contain the current EntityIndex
            return (m_SpanStorage[t_IndexSeq][m_CurrentEntity] && ...);
        }

        bool AreAllComponentsValid() const
        {
            return AreAllComponentsValidImpl(std::make_index_sequence<t_NumComponents>{});
        }


    public:
        //==============================
        // Comparison Operator(s)
        //==============================
        bool operator==(const FlatIterator& other) const
        {
            if constexpr (t_NumComponents == 1)
            {
                return m_SpanStorage.data() == other.m_SpanStorage.data() &&
                 m_CurrentEntity == other.m_CurrentEntity;
                
            }
            else
            {
                return SpanDataEqual(other) &&
                    m_CurrentEntity == other.m_CurrentEntity;
            }
        }

    private:
        // Comparison helper(s)
        template<std::size_t... t_IndexPack>
        bool SpanDataEqualImpl(const FlatIterator& other, std::index_sequence<t_IndexPack...>) const
        {
            // Fold over && to ensure all spans have equal .data()
            return ((m_SpanStorage[t_IndexPack].data() == other.m_SpanStorage[t_IndexPack].data()) && ...);
        }

        bool SpanDataEqual(const FlatIterator& other) const
        {
            // Create index sequence to iterate through all spans (0 to t_NumComponents - 1)
            return SpanDataEqualImpl(other, std::make_index_sequence<t_NumComponents>{});
        }
    private:
        //==============================
        // Internal Fields
        //==============================
        FlatStorage_t<t_NumComponents> m_SpanStorage{};
        EntityID m_CurrentEntity{ 0 };
    };

    template<size_t t_NumComponents>
	class FlatView
	{
    public:
        //==============================
        // Metaprogramming Types & Asserts
        //==============================
        static_assert(t_NumComponents != 0);
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		FlatView() = default;
		FlatView(FlatStorage_t<t_NumComponents> isEntityValidList) :
			m_SpanStorage(isEntityValidList) {};
		~FlatView() = default;
	public:
		//==============================
		// Enable For-Loop / Iterator Usage
		//==============================
		FlatIterator<t_NumComponents> begin() const
		{
			return FlatIterator<t_NumComponents>(m_SpanStorage, 0);
		};
		FlatIterator<t_NumComponents> end() const
		{
            if constexpr (t_NumComponents == 1)
            {
                return FlatIterator<t_NumComponents>(m_SpanStorage, m_SpanStorage.size());
            }
            else
            {
                return FlatIterator<t_NumComponents>(m_SpanStorage, m_SpanStorage[0].size());
            }
		};
	private:
		//==============================
		// Internal Fields
		//==============================
        FlatStorage_t<t_NumComponents> m_SpanStorage;
	};
}