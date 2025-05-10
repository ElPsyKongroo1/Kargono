#pragma once

#include <cstdint>
#include <cstdlib>

namespace Kargono::Memory
{
	constexpr size_t MEM_ALIGN_4_KIB = 4 * 1024;
	constexpr size_t MEM_ALIGN_2_MIB = 2 * 1024 * 1024;
	constexpr size_t MEM_ALIGN_1_GIB = 1 * 1024 * 1024 * 1024;

	class System
	{
	public:
		static uint8_t* GenAlloc(size_t dataSize, size_t alignment);
		static uint8_t* PageAlloc(size_t dataSize, size_t alignment);
	};
}




