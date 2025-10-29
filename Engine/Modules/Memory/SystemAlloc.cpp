#include "kgpch.h"

#include "Kargono/Memory/MemoryCommon.h"
#include "Kargono/Memory/SystemAlloc.h"

namespace Kargono::Memory
{
	uint8_t* System::GenAlloc(size_t dataSize, size_t alignment)
	{
		KG_ASSERT(Utility::IsPowerOfTwo(alignment));

		size_t upper_bound = dataSize + (alignment - 1);

		uintptr_t ptr = (uintptr_t) malloc(upper_bound);
		uintptr_t res = Utility::AlignForward(ptr, alignment);

		return (uint8_t*) res;
	}

	uint8_t* System::PageAlloc(size_t dataSize, size_t alignment)
	{
		if (alignment < k_MemAlign4KIB)
		{
			alignment = k_MemAlign4KIB;
		}

		return PageAllocHelper(dataSize, alignment);
	}
}
