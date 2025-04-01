#include "kgpch.h"

#include "Kargono/Memory/SystemAlloc.h"

#define ALIGN_PREV(v, align) ((v) & ~((align) - 1))
#define ALIGN_NEXT(v, align) ALIGN_PREV((v) + ((align) - 1), (align))

namespace Kargono::Memory
{
	uint8_t* System::GenAlloc(size_t dataSize, size_t alignment)
	{
		// TODO: Look into VirtualAlloc() and mmap() for bigger allocations
		uintptr_t returnPtr = (uintptr_t)malloc(dataSize + alignment);

		returnPtr = ALIGN_NEXT(returnPtr, alignment);

		return (uint8_t*)returnPtr;
	}
}

#undef ALIGN_PREV
#undef ALIGN_NEXT
