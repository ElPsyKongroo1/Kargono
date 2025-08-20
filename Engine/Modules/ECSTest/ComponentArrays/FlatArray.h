#pragma once

#include "Modules/ECSTest/ComponentArrays/IComponentStoreTest.h"
#include "Modules/ECSTest/Views/FlatView.h"

#include <array>
#include <vector>

namespace Kargono::ECS
{
	class FlatArray : public IComponentStore
	{
	private:
		//==============================
		// Constructors/Destructors
		//==============================
		FlatArray() = default;
		~FlatArray() = default;
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		void Init(EntityRegistryTest* entityRegistry, Memory::IAllocator* regAlloc,
				size_t componentSize, size_t componentAlignment) override
		{
			// Ensure dependencies are valid
			KG_ASSERT(entityRegistry);
			KG_ASSERT(regAlloc);
			i_EntityRegistry = entityRegistry;
			i_RegistryAlloc = regAlloc;

			AllocateBuffer(componentSize, componentAlignment);
		}

		void Terminate() override
		{
			if (m_ComponentBuffer)
			{
				DeallocateBuffer(m_ComponentSize, m_ComponentAlignment);
			}
		}

	private:
		bool AllocateBuffer(size_t componentSize, size_t componentAlignment)
		{
			KG_ASSERT(i_RegistryAlloc);
			KG_ASSERT(componentSize > 0);
			KG_ASSERT(componentAlignment > 0);
			KG_ASSERT(k_MaxEntities > 0);

			uint8_t* buffer{ i_RegistryAlloc->AllocRaw(componentSize * k_MaxEntities, componentAlignment) };

			if (!buffer)
			{
				return false;
			}

			m_ComponentSize = componentSize;
			m_ComponentAlignment = componentAlignment;
			m_ComponentBuffer = buffer;
			return true;
		}

		bool DeallocateBuffer(size_t componentSize, size_t componentAlignment)
		{
			KG_ASSERT(i_RegistryAlloc);
			KG_ASSERT(componentSize > 0);
			KG_ASSERT(componentAlignment > 0);
			KG_ASSERT(k_MaxEntities > 0);

			bool success { i_RegistryAlloc->DeallocRaw(m_ComponentBuffer, componentAlignment) };

			if (!success)
			{
				return false;
			}

			m_ComponentSize = 0;
			m_ComponentAlignment = 0;
			m_ComponentBuffer = nullptr;
			return true;
		}
	public:
		//==============================
		// Manage Components
		//==============================
		bool InsertComponent(EntityID entityID, void* component) override
		{
			KG_ASSERT(i_RegistryAlloc);
			KG_ASSERT(m_ComponentSize > 0);
			KG_ASSERT(k_MaxEntities > 0);

			// Ensure no component exists already
			bool isValid{ m_ValidEntityArray[entityID] };
			if (isValid)
			{
				return false;
			}

			uint8_t* destination{ &m_ComponentBuffer[entityID * m_ComponentSize] };

			// Update component & valid arrays
			memcpy(destination, component, m_ComponentSize);
			m_ValidEntityArray[entityID] = true;
			m_ComponentCount++;

			return true;
		}
		bool RemoveComponent(EntityID entityID) override
		{
			// Ensure a valid component exists
			bool isValid{ m_ValidEntityArray[entityID] };
			if (!isValid)
			{
				return false;
			}

			// Update valid array
			m_ValidEntityArray[entityID] = false;
			m_ComponentCount--;

			return true;
		}
		void* GetComponent(EntityID entityID) override
		{
			// Ensure a valid component exists
			bool isValid{ m_ValidEntityArray[entityID] };
			if (!isValid)
			{
				return nullptr;
			}

			// Update valid array
			return (void*)&m_ComponentBuffer[entityID * m_ComponentSize];
		}

		std::span<bool> GetValidEntityArray()
		{
			return m_ValidEntityArray;
		}
	private:
		//==============================
		// Internal Fields
		//==============================
		// Data array(s)
		uint8_t* m_ComponentBuffer{ nullptr };
		std::array<bool, k_MaxEntities> m_ValidEntityArray{};

		// Component info
		size_t m_ComponentSize{ 0 };
		size_t m_ComponentAlignment{ 0 };
		ComponentCount m_ComponentCount{ 0 };

		//==============================
		// Injected Section
		//==============================
		EntityRegistryTest* i_EntityRegistry{ nullptr };
		Memory::IAllocator* i_RegistryAlloc{ nullptr };
	private:
		//==============================
		// Owning Class(s)
		//==============================
		friend class Registry;
		friend class ComponentRegistry;
	};
}

