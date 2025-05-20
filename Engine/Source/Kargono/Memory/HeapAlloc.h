#pragma once
#include "Kargono/Memory/IAllocator.h"

#include <cstdint>
#include <cstddef>
#include <memory>
#include <unordered_set>

namespace Kargono::Memory
{
    struct AllocationInfo
    {
        //==============================
        // Public Fields
        //==============================
        uintptr_t m_Ptr{};
        size_t m_Alignment{ 0 };

        //==============================
        // Operator Overload(s)
        //==============================
        bool operator==(const AllocationInfo& other) const
        {
            return m_Ptr == other.m_Ptr && m_Alignment == other.m_Alignment;
        }
    };
}

// Don't mind this hashing garbage...
namespace std
{
    template<>
    struct hash<Kargono::Memory::AllocationInfo>
    {
        std::size_t operator()(const Kargono::Memory::AllocationInfo& info) const
        {
            size_t h1 = std::hash<uintptr_t>{}(info.m_Ptr);
            size_t h2 = std::hash<size_t>{}(info.m_Alignment);
            return h1 ^ (h2 << 1);  // Combine hashes
        }
    };
}

namespace Kargono::Memory
{


    // TODO: Could have a lot of improvements. Meant to be a simple proof of concept for interface
    class HeapAllocator : public IAllocator
    {
    public:
        //==============================
        // Constructors/Destructors
        //==============================
        HeapAllocator() = default;
        ~HeapAllocator() override = default;

        //==============================
        // Raw Allocation API
        //==============================
        virtual uint8_t* AllocRaw(size_t dataSize, size_t alignment) override;
        virtual bool DeallocRaw(uint8_t* dataPtr, size_t alignment) override;

        //==============================
        // Reset Allocation
        //==============================
        virtual void Reset() override;
    private:
        //==============================
        // Internal Fields
        //==============================
        // TODO: Could be replaced with pointer tagging??? Here just to get working....
        std::unordered_set<AllocationInfo> m_ActiveAllocs{};
    };
}

