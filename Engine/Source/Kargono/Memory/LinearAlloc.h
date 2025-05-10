#pragma once

#include <cstdint>

#include <new>
#include <utility>

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
			uint8_t* buffer = AllocRaw(sizeof(Type) * Count, Align);
			if (buffer == nullptr)
				return nullptr;

			// Construct objects in-place with the given args
			return new (buffer) Type(std::forward<decltype(args)>(args)...);
		}

	public:
		//==============================
		// Manage Allocator
		//==============================
		void Reset();

		// TODO: Optionally add resize allocation method here: https://www.gingerbill.org/article/2019/02/08/memory-allocation-strategies-002/

	private:
		uint8_t* m_Buffer{ nullptr };
		size_t m_BufferSize{ 0 };
		size_t m_Offset{ 0 };
	};
}
