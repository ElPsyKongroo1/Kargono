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

		// Return a type
		template<typename Type, size_t Count = 1, size_t Align = alignof(Type)>
		Type* Alloc(auto&&... args)
		{
			Type* returnPtr = new (AllocRaw(sizeof(Type) * Count, Align)) Type(std::forward<decltype(args)>(args)...);
			return returnPtr;
		}

		/*template<typename T, size_t N>
		T* Alloc(size_t size = sizeof(T) * N, size_t align = alignof(T))
		{
			return new (Alloc(size, align)) T();
		}*/

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
