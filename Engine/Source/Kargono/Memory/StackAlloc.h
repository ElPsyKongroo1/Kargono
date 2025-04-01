#pragma once

#include <utility>
#include <cstdint>

namespace Kargono::Memory
{
	struct StackAllocHeader
	{
		size_t m_PreviousOffset;
		size_t m_Padding;
	};

	class StackAlloc
	{
	public:
		//==============================
		// Constuctors/Destructors
		//==============================
		StackAlloc() = default;
		~StackAlloc() = default;
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		void Init(uint8_t* backingBuffer, size_t bufferSize);
		void Terminate();

		//==============================
		// Allocate Memory
		//==============================
		// Allocate uninitialized bytes
		uint8_t* AllocRaw(size_t dataSize, size_t alignment);

		// Allocate memory for specified type
		template<typename Type, size_t Count = 1, size_t Align = alignof(Type)>
		Type* Alloc(auto&&... args)
		{
			// Resource for placement new: https://www.geeksforgeeks.org/placement-new-operator-cpp/

			// Allocate memory for one or more objects of the specified type
			Type* returnPtr = new (AllocRaw(sizeof(Type) * Count, Align)) Type(std::forward<decltype(args)>(args)...);

			// Return pointer to memory (AllocRaw could return nullptr)
			return returnPtr;
		}

	private:
		// Allocation helper functions
		uintptr_t AlignHeaderPadding(uintptr_t pointer, size_t alignment, size_t headerSize);

	public:
		//==============================
		// De-Allocate Memory
		//==============================
		void Free(uint8_t* dataPtr);
	public:
		//==============================
		// Manage Allocator
		//==============================
		void Reset();

	private:
		uint8_t* m_Buffer{ nullptr };
		size_t m_BufferSize{ 0 };
		size_t m_CurrentOffset{ 0 };
		size_t m_PreviousOffset{ 0 };
	};
}
