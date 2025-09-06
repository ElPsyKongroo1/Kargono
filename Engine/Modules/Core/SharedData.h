#pragma once
#include <Kargono/Core/Base.h>
#include <Modules/FileSystem/FileSystem.h>
#include "Kargono/Memory/IAllocator.h"

#include <cstdint>
#include <unordered_map>

namespace Kargono
{
	class SharedDataRegistry;

	class SharedData
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		SharedData() = default;
		SharedData(uint8_t* data, size_t size, size_t alignment);
		~SharedData() = default;

	public:
		//==============================
		// Operator Overloads
		//==============================
		operator bool() const 
		{
			return (bool)m_Data && m_DataSize > 0; 
		}

		//==============================
		// Getters/Setters
		//==============================
		template<typename T>
		T* As()
		{
			KG_ASSERT(m_Data);
			KG_ASSERT(m_DataSize == sizeof(T));
			KG_ASSERT(m_Alignment == alignof(T));

			return static_cast<T*>(m_Data);
		}

		size_t Size() const
		{
			return m_DataSize;
		}

	private:
		//==============================
		// Internal Fields
		//==============================
		uint8_t* m_Data{ nullptr };
		size_t m_DataSize{ 0 };
		size_t m_Alignment{ 0 };

	private:
		friend class SharedDataRegistry;
	};

	using SharedDataID = uint32_t;

	class SharedDataRegistry
	{
		using RegistryMap = std::unordered_map<SharedDataID, SharedData>;
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		SharedDataRegistry() = default;
		~SharedDataRegistry()
		{
			// Ensure terminate was called
			KG_ASSERT(!m_Active);
		}

	public:
		//==============================
		// Lifecycle Functions
		//==============================
		[[nodiscard]] bool Init(Memory::IAllocator* backingAllocator)
		{
			KG_ASSERT(!m_Active);

			// Set up backing allocator
			KG_ASSERT(backingAllocator);
			i_Allocator = backingAllocator;

			// Set registry to active
			m_Active = true;
			return m_Active;
		}

		[[nodiscard]] bool Terminate()
		{
			KG_ASSERT(m_Active);
			
			// Clean up allocated memory
			i_Allocator->Reset();
			m_DataRegistry.clear();

			// Clear injected resources
			i_Allocator = nullptr;

			// Set registry to in-active
			m_Active = false;
			return m_Active;
		}
	public:
		//==============================
		// Register Data
		//==============================
		template<typename T>
		[[nodiscard]] bool Register(SharedDataID dataID)
		{
			KG_ASSERT(m_Active);

			// Allocate memory for the new data and call default constructor
			T* returnedMemory = i_Allocator->Alloc<T>();

			// Check for failed memory allocation
			if (!returnedMemory)
			{
				return false;
			}

			// Register the shared data
			auto [it, success] = m_DataRegistry.emplace(dataID, SharedData(static_cast<uint8_t*>(returnedMemory), sizeof(T), alignof(T)));

			// Handle failure of insertion
			if (!success)
			{
				bool deallocSuccess = i_Allocator->Dealloc<T>(returnedMemory);

				// This should not occur if original allocation succeeded
				KG_ASSERT(deallocSuccess);
			}

			return success;
		}
		//==============================
		// Access Data
		//==============================
		template<typename T>
		[[nodiscard]] ExpectedRef<T> Get(SharedDataID dataID)
		{
			KG_ASSERT(m_Active);

			RegistryMap::iterator iter = m_DataRegistry.find(dataID);

			// Handle failure case
			if (iter == m_DataRegistry.end())
			{
				return {};
			}

			// Ensure the shared data works with the provided type
			SharedData& data = iter->second;

			// Handle find success
			return std::ref(*data.As<T>());
		}


		[[nodiscard]] ExpectedRef<SharedData> GetSharedData(SharedDataID dataID)
		{
			KG_ASSERT(m_Active);

			RegistryMap::iterator iter = m_DataRegistry.find(dataID);

			// Handle failure case
			if (iter == m_DataRegistry.end())
			{
				return {};
			}

			// Handle find success
			return iter->second;
		}

		//==============================
		// Remove Data
		//==============================
		[[nodiscard]] bool RemoveSharedData(SharedDataID dataID)
		{
			KG_ASSERT(m_Active);

			// Ensure dataID leads to data
			if (!m_DataRegistry.contains(dataID))
			{
				return false;
			}

			// Deallocate the indicated memory
			SharedData& sharedData = m_DataRegistry[dataID];
			bool deallocSuccess = i_Allocator->DeallocRaw(sharedData.m_Data, sharedData.m_Alignment);
			KG_ASSERT(deallocSuccess);

			// Remove the shared data entry
			size_t numErased = m_DataRegistry.erase(dataID);

			// Catch case where duplicates are found
			KG_ASSERT(numErased < 2);
			return numErased > 0; 
		}
	private:
		//==============================
		// Internal Fields
		//==============================
		RegistryMap m_DataRegistry{};
		bool m_Active{ false };

		//==============================
		// Injected Dependencies
		//==============================
		Memory::IAllocator* i_Allocator{ nullptr };
	};
}

namespace Kargono::Utility
{
	consteval SharedDataID GetSharedDataID(const char* dataName)
	{
		return static_cast<SharedDataID>(FileSystem::CRCFromString(dataName));
	}
}
