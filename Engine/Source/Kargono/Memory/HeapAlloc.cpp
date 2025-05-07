#include "kgpch.h"

#include "Kargono/Memory/HeapAlloc.h"

namespace Kargono
{
	uint8_t* HeapAllocator::AllocRaw(size_t dataSize, size_t alignment)
    {
        // Use aligned allocation (C++17 and up)
        void* ptr = ::operator new(dataSize, std::align_val_t(alignment));
        return static_cast<uint8_t*>(ptr);
    }
    bool HeapAllocator::DeallocRaw(uint8_t* dataPtr, size_t alignment)
    {
        if (!dataPtr)
        {
            return false;
        }

        // Deallocate the memory
        ::operator delete(dataPtr, std::align_val_t(alignment));
        return true;
    }
}