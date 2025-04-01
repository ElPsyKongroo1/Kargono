#pragma once
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
}
