#pragma once

#include <cstdint>
#include <cstdlib>

#define ALIGN_PREV(v, align) ((v) & ~((align) - 1))
#define ALIGN_NEXT(v, align) ALIGN_PREV((v) + ((align) - 1), (align))

namespace Kargono::Memory
{
	inline uint8_t* Allocate(size_t dataSize, size_t alignment)
	{
		// TODO: We are not using alli
		// TODO: Look into VirtualAlloc() and mmap() for bigger allocations
		uintptr_t returnPtr = (uintptr_t)malloc(dataSize + alignment);

		returnPtr = ALIGN_NEXT(returnPtr, alignment);

		return (uint8_t*)returnPtr;
	}
}

#undef ALIGN_PREV
#undef ALIGN_NEXT


