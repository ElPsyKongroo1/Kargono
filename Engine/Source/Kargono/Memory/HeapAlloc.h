#pragma once
#include "Kargono/Memory/IAllocator.h"

#include <cstdint>
#include <cstddef>
#include <memory>

namespace Kargono
{
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
    };
}

