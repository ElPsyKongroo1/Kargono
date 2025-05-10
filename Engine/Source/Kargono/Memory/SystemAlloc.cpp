#include "kgpch.h"

#include "Kargono/Memory/MemoryCommon.h"
#include "Kargono/Memory/SystemAlloc.h"

namespace Kargono::Memory
{
	uint8_t* System::GenAlloc(size_t dataSize, size_t alignment)
	{
		KG_ASSERT(Utility::IsPowerOfTwo(alignment));

		size_t upper_bound = dataSize + (alignment - 1);

		// TODO: Look into VirtualAlloc() and mmap() for bigger allocations
		uintptr_t ptr = (uintptr_t) malloc(upper_bound);
		uintptr_t res = Utility::AlignForward(ptr, alignment);

		return (uint8_t*) res;
	}
}
