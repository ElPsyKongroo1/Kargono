#pragma once

#include <utility>
#include <cstdint>

namespace Kargono::Memory
{
	struct PoolFreeNode
	{
		PoolFreeNode* m_NextNode{ nullptr };
	};

	class PoolAlloc
	{
	public:
		//==============================
		// Constuctors/Destructors
		//==============================
		PoolAlloc() = default;
		~PoolAlloc() = default;
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		void Init(uint8_t* backingBuffer, size_t bufferSize, size_t chunkSize, size_t chunkAlignment);

		template<typename Type>
		void Init(uint8_t* backingBuffer, size_t bufferSize)
		{
			Init(backingBuffer, bufferSize, sizeof(Type), alignof(Type));
		}
		void Terminate();

		//==============================
		// Allocate Memory
		//==============================
		// Allocate uninitialized bytes
		uint8_t* AllocRaw();

		// Allocate memory for specified type
		template<typename Type>
		Type* Alloc(auto&&... args)
		{
			// Resource for placement new: https://www.geeksforgeeks.org/placement-new-operator-cpp

			KG_ASSERT(sizeof(Type) == m_ChunkSize);

			// Allocate memory for one or more objects of the specified type
			Type* returnPtr = new (AllocRaw()) Type(std::forward<decltype(args)>(args)...);

			// Return pointer to memory (AllocRaw could return nullptr)
			return returnPtr;
		}

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
		size_t m_ChunkSize{ 0 };

		PoolFreeNode* m_HeadNode{ nullptr };
	};
}
