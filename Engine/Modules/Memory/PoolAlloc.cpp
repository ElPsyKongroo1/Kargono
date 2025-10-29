#include "kgpch.h"
#include "Kargono/Memory/PoolAlloc.h"

#include "Kargono/Memory/MemoryCommon.h"

namespace Kargono::Memory
{
	void PoolAlloc::Init(uint8_t* backingBuffer, size_t bufferSize, size_t chunkSize, size_t chunkAlignment)
	{
		// Align the backing buffer to the chunk's alignment
		uintptr_t initialStart = (uintptr_t)backingBuffer;
		uintptr_t alignedStart = Utility::AlignForward(initialStart, (uintptr_t)chunkAlignment);
		bufferSize -= (size_t)(alignedStart - initialStart);

		// Align the chunk size up to the required chunk alignment
		chunkSize = Utility::AlignForward(chunkSize, chunkAlignment);

		// Ensure the parameters are valid
		KG_ASSERT(chunkSize >= sizeof(PoolFreeNode), "Chunk size is too small for pool");
		KG_ASSERT(bufferSize >= chunkSize, "Buffer size is too small for the chunk size");

		// Store the adjusted parameters
		m_Buffer = (uint8_t*)backingBuffer;
		m_BufferSize = bufferSize;
		m_ChunkSize = chunkSize;
		m_HeadNode = nullptr;

		// Set entire buffer to free list nodes
		Reset();
	}

	void PoolAlloc::Terminate()
	{
		m_Buffer = nullptr;
		m_BufferSize = 0;
		m_ChunkSize = 0;
		m_HeadNode = nullptr;
	}

	uint8_t* PoolAlloc::AllocRaw()
	{
		PoolFreeNode* currentNode = m_HeadNode;

		KG_ASSERT(currentNode);

		m_HeadNode = m_HeadNode->m_NextNode;

		return (uint8_t*)currentNode;
	}

	void PoolAlloc::Free(uint8_t* dataPtr)
	{
		PoolFreeNode* node;

		// Get pool bounds
		uint8_t* dataStart = m_Buffer;
		uint8_t* dataEnd = &m_Buffer[m_BufferSize];

		// Ensure data pointer is valid for this pool
		if (dataPtr == nullptr)
		{
			return;
		}
		KG_ASSERT(dataStart <= dataPtr && dataPtr < dataEnd, "Data pointer is out of bounds for this pool");

		// Push new node onto free list
		node = (PoolFreeNode*)dataPtr;
		node->m_NextNode = m_HeadNode;
		m_HeadNode = node;
	}

	void PoolAlloc::Reset()
	{
		size_t chunkCount = m_BufferSize / m_ChunkSize;

		for (size_t i { 0 }; i < chunkCount; i++)
		{
			uint8_t* dataPtr = &m_Buffer[i * m_ChunkSize];

			// Cast chunk into a free node
			PoolFreeNode* node = (PoolFreeNode*)dataPtr;

			// Push node onto free list
			node->m_NextNode = m_HeadNode;
			m_HeadNode = node;
		}
	}

}
