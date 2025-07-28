#pragma once

#include "Modules/ECSTest/Views/IViewTest.h"

namespace Kargono::ECS
{
    class FlatIterator
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
        explicit FlatIterator() = default;
        explicit FlatIterator(std::span<bool> validEntitySpan, size_t index)
            : m_IsEntityValidSpan{ validEntitySpan }, m_CurrentIndex(index) 
        {
            SkipToNextValid();
        }
    public:
        //==============================
        // Retrieve Data
        //==============================
        EntityID operator*() const
        {
            KG_ASSERT(m_CurrentIndex < m_IsEntityValidSpan.size());
            return m_CurrentIndex;
        }

    public:
        //==============================
        // Advance Iterator
        //==============================
        FlatIterator& operator++()
        {
            ++m_CurrentIndex;
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
            while (m_CurrentIndex < m_IsEntityValidSpan.size() &&
                !m_IsEntityValidSpan[m_CurrentIndex])
            {
                ++m_CurrentIndex;
            }
        }

    public:
        //==============================
        // Comparison Operator(s)
        //==============================
        bool operator==(const FlatIterator& other) const
        {
            return m_IsEntityValidSpan.data() == other.m_IsEntityValidSpan.data() &&
                m_CurrentIndex == other.m_CurrentIndex;
        }
    private:
        //==============================
        // Internal Fields
        //==============================
        std::span<bool> m_IsEntityValidSpan{};
        EntityID m_CurrentIndex{ 0 };
    };

	class FlatView
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		FlatView() = default;
		FlatView(std::span<bool> isEntityValidList) :
			m_IsEntityValidSpan(isEntityValidList) {};
		~FlatView() = default;
	public:
		//==============================
		// Enable For-Loop / Iterator Usage
		//==============================
		FlatIterator begin() const
		{
			return FlatIterator(m_IsEntityValidSpan, 0);
		};
		FlatIterator end() const
		{
			return FlatIterator(m_IsEntityValidSpan, m_IsEntityValidSpan.size());
		};
	private:
		//==============================
		// Internal Fields
		//==============================
		std::span<bool> m_IsEntityValidSpan;
	};
}