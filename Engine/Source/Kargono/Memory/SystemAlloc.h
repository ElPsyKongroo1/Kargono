#pragma once

#include <cstdint>
#include <cstdlib>

namespace Kargono::Memory
{
	class System
	{
	public:
		static uint8_t* GenAlloc(size_t dataSize, size_t alignment);
		static uint8_t* PageAlloc(size_t dataSize, size_t alignment);
	};
}




