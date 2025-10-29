#pragma once

#include "Kargono/Memory/Platform/MemoryTypes.h"

#include <cstdint>
#include <cstdlib>


namespace Kargono::Memory
{
	constexpr size_t k_MemAlign4KIB = 4 * 1024;
	constexpr size_t k_MemAlign2MIB = 2 * 1024 * 1024;
	constexpr size_t k_MemAlign1GIB = 1 * 1024 * 1024 * 1024;

	struct MirrorAllocResult
	{
		MemoryFileDesc m_MemoryFileDesc;
		uint8_t* m_Data;
		size_t m_Length;
		size_t m_Capacity;
	};

	class System
	{
	public:
		static uint8_t* GenAlloc(size_t dataSize, size_t alignment);
		static uint8_t* PageAlloc(size_t dataSize, size_t alignment);
		static MirrorAllocResult MirrorAlloc(size_t dataSize, size_t mirrors);
	private:
		static uint8_t* PageAllocHelper(size_t dataSize, size_t alignment);
	};
}




