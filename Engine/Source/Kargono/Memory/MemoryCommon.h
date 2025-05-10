#pragma once

#include <cstddef>
#include <cstdint>

namespace Kargono::Utility
{
	//==============================
	// Memory Utility Function(s)
	//==============================
	inline bool IsPowerOfTwo(uintptr_t x)
	{
		return (x & (x - 1)) == 0;
	}

	inline uintptr_t AlignBackward(uintptr_t pointer, size_t alignment)
	{
		KG_ASSERT(IsPowerOfTwo(alignment));

		uintptr_t mask = ~(alignment - 1);
		uintptr_t res = pointer & mask;

		return res;
	}

	inline uintptr_t AlignForward(uintptr_t pointer, size_t alignment)
	{
		// Most/all architectures use power-of-two alignment
		KG_ASSERT(IsPowerOfTwo(alignment));

		uintptr_t upper_bound = pointer + (alignment - 1);
		uintptr_t res = AlignBackward(upper_bound, alignment);

		return res;
	}
}
