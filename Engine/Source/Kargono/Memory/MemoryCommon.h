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

	inline uintptr_t AlignForward(uintptr_t pointer, size_t alignment)
	{
		uintptr_t returnPtr;
		uintptr_t alignmentInt;
		uintptr_t modulo;

		// Most/all architectures use power-of-two alignment
		KG_ASSERT(Utility::IsPowerOfTwo((uintptr_t)alignment));

		returnPtr = pointer;
		alignmentInt = alignment;

		// Bitwise operation to calculate modulo for powers of two
		modulo = returnPtr & (alignmentInt - 1);

		// Move the pointer by the additive inverse if a modulo (remainder) exists
		if (modulo != 0)
		{
			returnPtr += alignmentInt - modulo;
		}

		return returnPtr;
	}
}
