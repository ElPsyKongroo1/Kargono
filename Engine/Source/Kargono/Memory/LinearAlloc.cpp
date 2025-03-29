#include "kgpch.h"
#include "LinearAlloc.h"

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
	}

	uint8_t* LinearAlloc::AllocRaw(size_t dataSize, size_t /*alignment*/)
	{
		if (m_Offset + dataSize <= m_BufferSize)
		{
			// Allocate the memory!
			uint8_t* returnVal = m_Buffer + m_Offset;
			m_Offset += dataSize;
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
