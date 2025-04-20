#pragma once

#include <iterator>
#include <type_traits>
#include <concepts>

//==============================
// Iterator Concepts
//==============================
// Ensure comparison function is valid and returns bool
template<typename Derived, typename DataType>
concept HasCompareFunc = requires(Derived & derived, DataType * ptr)
{
	{ derived.Compare(ptr) } -> std::convertible_to<bool>;
};

//==============================
// Usage:
//struct CustomIterator : public Iterator<CustomIterator, int>
//{
// // Optionally define this
//	bool Compare(int* ptr)
// {
//	 return (custom compare expression)
// }
//};
//==============================

template<typename Derived, typename ContainerType, typename DataType>
struct Iterator
{
public:
	//==============================
	// Constructor/Destructors
	//==============================
	Iterator(ContainerType* container, DataType* data) : m_ContainerPtr(container), m_Ptr(data)  {}

public:
	//==============================
	// Operator Overloads
	//==============================
	// Get reference/pointer
	DataType& operator*() const
	{
		return *m_Ptr;
	}
	DataType* operator->()
	{
		return m_Ptr;
	}
	// Increment operators
	Iterator& operator++() // Prefix operator (++iter)
	{
		++m_Ptr;

		// Check if derived class provides Compare function
		if constexpr (HasCompareFunc<Derived, DataType>)
		{
			// Increment the pointer if comparison succeeds
			while (static_cast<Derived*>(this)->Compare(m_Ptr))
			{
				++m_Ptr;
			}
		}
	
		return *this;
	}
	Iterator operator++(int) // Postfix operator (iter++)
	{
		// Store the original iterator value to be returned
		Iterator tmp = *this;

		++(*this);

		// Check if derived class provides Compare function
		if constexpr (HasCompareFunc<Derived, DataType>)
		{
			// Increment the pointer if comparison succeeds
			while (static_cast<Derived*>(this)->Compare(m_Ptr))
			{
				++(*this);
			}
		}

		// Return the original iter
		return tmp;
	}
	// Logical operators
	friend bool operator== (const Iterator& iterA, const Iterator& iterB)
	{
		return iterA.m_Ptr == iterB.m_Ptr;
	};
	friend bool operator!= (const Iterator& iterA, const Iterator& iterB)
	{
		return iterA.m_Ptr != iterB.m_Ptr;
	};
protected:
	//==============================
	// Internal Fields
	//==============================
	DataType* m_Ptr{ nullptr };
	ContainerType* m_ContainerPtr{ nullptr };
};
