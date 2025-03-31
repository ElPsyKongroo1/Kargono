#pragma once

#include <utility>
#include <cstdint>

namespace Kargono::Memory
{
	class LinearAlloc
	{
	public:
		//==============================
		// Constuctors/Destructors
		//==============================
		LinearAlloc() = default;
		~LinearAlloc() = default;
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

		// TODO: Optionally add resize allocation method here: https://www.gingerbill.org/article/2019/02/08/memory-allocation-strategies-002/

	private:
		bool IsPowerOfTwo(uintptr_t x);
		uintptr_t AlignForward(uintptr_t pointer, size_t alignment);

	public:
		//==============================
		// Manage Allocator
		//==============================
		void Reset();

	private:
		uint8_t* m_Buffer{ nullptr };
		size_t m_BufferSize{ 0 };
		size_t m_Offset{ 0 };
	};
}
