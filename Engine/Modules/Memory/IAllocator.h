#pragma once
#include <cstdint>

namespace Kargono::Memory
{
	class IAllocator
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		IAllocator() = default;
		virtual ~IAllocator() = default;

	public:
		//==============================
		// Raw Allocation API (No In-Place Construction)
		//==============================
		[[nodiscard]] virtual uint8_t* AllocRaw(size_t dataSize, size_t alignment) = 0;
		[[nodiscard]] virtual bool DeallocRaw(uint8_t* dataPtr, size_t alignment) = 0;

		//==============================
		// Type Allocation & Construction API
		//==============================
		template<typename t_Type, size_t t_Align = alignof(t_Type)>
		[[nodiscard]] t_Type* Alloc(auto&&... args)
		{
			// Allocate memory
			uint8_t* rawAlloc = AllocRaw(sizeof(t_Type), t_Align);
			if (!rawAlloc)
			{
				return nullptr;
			}

			// Construct the object in-place
			// Resource for placement new: https://www.geeksforgeeks.org/placement-new-operator-cpp/

			// Return pointer to memory (AllocRaw could return nullptr)
			return new (rawAlloc) t_Type(std::forward<decltype(args)>(args)...);
		}

		template<typename t_Type, size_t t_Align = alignof(t_Type)>
		[[nodiscard]] bool Dealloc(t_Type* dataPtr)
		{
			if (!dataPtr)
			{
				return false;
			}

			// Call the destructor
			dataPtr->~t_Type();

			// Free the underlying memory
			return DeallocRaw(static_cast<uint8_t*>(dataPtr), t_Align);
		}

		//==============================
		// Reset Allocator
		//==============================
		virtual void Reset() = 0;
	};
}