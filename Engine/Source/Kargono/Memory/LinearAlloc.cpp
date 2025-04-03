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
		uintptr_t currentPtr = (uintptr_t)m_Buffer + (uintptr_t)m_Offset;

		// Get the pointer to the newly aligned memory
		uintptr_t offset = Utility::AlignForward(currentPtr, alignment);

		// Calculate the relative offset for the alignment pointer
		offset -= (uintptr_t)m_Buffer;

		if (offset + dataSize <= m_BufferSize)
		{
			// Allocate the memory!
			uint8_t* returnVal = m_Buffer + m_Offset;
			m_Offset = offset + dataSize;

			// Note that this allocator does not zero out the memory buffer per-allocation
			return returnVal;
		}

		// Failed to find enough memory in buffer
		return nullptr;
	}

	void LinearAlloc::Reset()
	{
		m_Offset = 0;
	}

}
