#include "kgpch.h"

#include "Kargono/Memory/LinearAlloc.h"
#include "Kargono/Memory/MemoryCommon.h"

namespace Kargono::Memory
{
	void LinearAlloc::Init(uint8_t* backingBuffer, size_t bufferSize)
	{
		m_Buffer = backingBuffer;
		m_BufferSize = bufferSize;
		m_Offset = 0;
	}

	void LinearAlloc::Terminate()
	{
		m_Buffer = nullptr;
		m_BufferSize = 0;
		m_Offset = 0;
	}

	uint8_t* LinearAlloc::AllocRaw(size_t dataSize, size_t alignment)
	{
		uintptr_t currentPtr = (uintptr_t) m_Buffer + m_Offset;
		uintptr_t endPtr = (uintptr_t) m_Buffer + m_BufferSize;

		// Get the pointer to the newly aligned memory
		uintptr_t alignedPtr = Utility::AlignForward(currentPtr, alignment);

		// If any part of the (alignedPtr + dataSize) overruns endPtr
		if (endPtr < alignedPtr + dataSize)
			return nullptr; // Failed to find enough memory in buffer

		// Otherwise: `this->length = (alignedPtr + dataSize) - this->ptr;`
		m_Offset = (alignedPtr + dataSize) - (uintptr_t) m_Buffer;

		// Note that this allocator does not zero out the memory buffer per-allocation
		return (uint8_t*) alignedPtr;
	}

	void LinearAlloc::Reset()
	{
		m_Offset = 0;
	}

}
