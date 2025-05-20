#include "kgpch.h"

#include "Kargono/Memory/HeapAlloc.h"

namespace Kargono::Memory
{
    uint8_t* HeapAllocator::AllocRaw(size_t dataSize, size_t alignment)
    {
        // Use aligned allocation (C++17 and up)
        void* ptr = ::operator new(dataSize, std::align_val_t(alignment));

        // Cache new allocation
        KG_ASSERT(m_ActiveAllocs.insert({ (uintptr_t)ptr, alignment }).second);
        return static_cast<uint8_t*>(ptr);
    }
    bool HeapAllocator::DeallocRaw(uint8_t* dataPtr, size_t alignment)
    {
        AllocationInfo info{ (uintptr_t)dataPtr, alignment };

        if (!dataPtr || !m_ActiveAllocs.contains(info))
        {
            return false;
        }

        // Deallocate the memory
        ::operator delete(dataPtr, std::align_val_t(alignment));

        // Remove the cached reference
        size_t numErased = m_ActiveAllocs.erase(info);

        return numErased > 0;
    }
    void HeapAllocator::Reset()
    {
        for (const AllocationInfo& allocInfo : m_ActiveAllocs)
        {
            ::operator delete
                (
                    reinterpret_cast<void*>(allocInfo.m_Ptr),
                    std::align_val_t(allocInfo.m_Alignment)
                    );
        }
        m_ActiveAllocs.clear();
    }
}