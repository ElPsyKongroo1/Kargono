#include "kgpch.h"

#include "Kargono/Memory/StackAlloc.h"

#include "Kargono/Memory/MemoryCommon.h"

namespace Kargono::Memory
{
	void StackAlloc::Init(uint8_t* backingBuffer, size_t bufferSize)
	{
		m_Buffer = backingBuffer;
		m_BufferSize = bufferSize;
		m_CurrentOffset = 0;
		m_PreviousOffset = 0;
	}
	void StackAlloc::Terminate()
	{

	}
	uint8_t* StackAlloc::AllocRaw(size_t dataSize, size_t alignment)
	{
		uintptr_t currentPtr, nextPtr;
		size_t padding;
		StackAllocHeader* header;

		// Most/all architectures use power-of-two alignment
		KG_ASSERT(Utility::IsPowerOfTwo((uintptr_t)alignment));

		// Truncate alignment to 128 since header can only address values
		// less than < 256
		if (alignment > 128)
		{
			alignment = 128;
		}

		// Get the old allocation pointer
		currentPtr = (uintptr_t)m_Buffer + m_CurrentOffset;

		// Generate aligned padding and ensure space
		padding = AlignHeaderPadding(currentPtr, alignment, sizeof(StackAllocHeader));
		if (m_CurrentOffset + padding + dataSize > m_BufferSize)
		{
			return nullptr;
		}

		m_PreviousOffset = m_CurrentOffset;
		m_CurrentOffset += padding;

		// Get the new allocation pointer
		nextPtr = currentPtr + (uintptr_t)padding;

		// Create/fill the header data
		header = (StackAllocHeader*)(nextPtr - sizeof(StackAllocHeader));
		header->m_Padding = padding;
		header->m_PreviousOffset = m_PreviousOffset;

		m_CurrentOffset += dataSize;
		return (uint8_t*)nextPtr;
	}
	uintptr_t StackAlloc::AlignHeaderPadding(uintptr_t pointer, size_t alignment, size_t headerSize)
	{
		#define PowerOfTwoModulo(dividend, divisor) dividend & (divisor - 1)

		uintptr_t pointerInt;
		uintptr_t alignmentInt;
		uintptr_t modulo;
		uintptr_t returnPadding;
		uintptr_t neededSpace;

		// Most/all architectures use power-of-two alignment
		KG_ASSERT(Utility::IsPowerOfTwo((uintptr_t)alignment));

		pointerInt = pointer;
		alignmentInt = alignment;

		// Bitwise operation to calculate modulo for powers of two
		modulo = PowerOfTwoModulo(pointerInt, alignmentInt);

		returnPadding = 0;
		neededSpace = 0;

		// Move the padding if a modulo (remainder) exists
		if (modulo != 0)
		{
			returnPadding += alignmentInt - modulo;
		}

		// Now we need to incorporate the header size into the padding
		neededSpace = (uintptr_t)headerSize;

		// If the header already fits in the padding, skip
		if (returnPadding < neededSpace)
		{
			// Needed space is now the remainder
			neededSpace -= returnPadding;
			
			// Add the needed space to the padding
			if ((PowerOfTwoModulo(neededSpace, alignmentInt)) == 0)
			{
				// Truncate the needed space based on the alignment
				returnPadding += alignmentInt * (neededSpace / alignmentInt);
			}
			else 
			{
				// Trunate the needed space based on alignment and 
				// overestimate the padding 
				returnPadding += alignmentInt * (1 + (neededSpace / alignmentInt));
			}
		}

		return (size_t)returnPadding;

		#undef PowerOfTwoModulo
	}
	void StackAlloc::Free(uint8_t* dataPtr)
	{
		if (dataPtr == nullptr) 
		{
			return;
		}

		uintptr_t start, end, currentPtr;
		StackAllocHeader* header;
		size_t previousOffset;

		// Get the bounds of the allocator and data location
		start = (uintptr_t)m_Buffer;
		end = start + (uintptr_t)m_BufferSize;
		currentPtr = (uintptr_t)dataPtr;

		// Ensure the data pointer is within bounds
		KG_ASSERT(start <= currentPtr && currentPtr < end);

		if (currentPtr >= start + (uintptr_t)m_CurrentOffset)
		{
			// Note this indicates a possible double free
			return;
		}

		// Get the header from the data pointer
		header = (StackAllocHeader*)(currentPtr - sizeof(StackAllocHeader));
		previousOffset = (size_t)(currentPtr - (uintptr_t)header->m_Padding - start);

		KG_ASSERT(previousOffset == header->m_PreviousOffset, "Out of order stack alloc free");

		// Move the data pointer back
		m_CurrentOffset = previousOffset;
		m_PreviousOffset = header->m_PreviousOffset;
	}
	void StackAlloc::Reset()
	{
		m_CurrentOffset = 0;
	}
}
