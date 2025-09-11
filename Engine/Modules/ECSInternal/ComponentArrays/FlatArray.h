#pragma once

#include "Modules/ECSInternal/ComponentArrays/IComponentStore.h"
#include "Modules/ECSInternal/Views/FlatView.h"

#include <array>
#include <vector>
#include <sstream>

namespace Kargono::ECSInternal
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
		void Init(EntityRegistry* entityRegistry, Memory::IAllocator* regAlloc,
				ComponentMetadata metadata) override
		{
			// Ensure dependencies are valid
			KG_ASSERT(entityRegistry);
			KG_ASSERT(regAlloc);
			i_EntityRegistry = entityRegistry;
			i_RegistryAlloc = regAlloc;
			m_CompMetadata = metadata;
			
			AllocateBuffer(m_CompMetadata.m_ComponentSize,
				m_CompMetadata.m_ComponentAlignment);

			Clear();
		}

		void Terminate() override
		{
			Clear();

			if (m_ComponentBuffer)
			{
				DeallocateBuffer(m_CompMetadata.m_ComponentSize,
					m_CompMetadata.m_ComponentAlignment);
			}

			i_EntityRegistry = nullptr;
			i_RegistryAlloc = nullptr;
			m_CompMetadata = {};
		}

		void Clear() override
		{
			ResetValidEntityArray();
			m_ComponentCount = 0;
		}

	private:
		// Helper functions
		void ResetValidEntityArray()
		{
			for (bool& valid : m_ValidEntityArray)
			{
				valid = false;
			}
		}

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

			m_CompMetadata.m_ComponentSize = componentSize;
			m_CompMetadata.m_ComponentAlignment = componentAlignment;
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

			m_CompMetadata.m_ComponentSize = 0;
			m_CompMetadata.m_ComponentAlignment = 0;
			m_ComponentBuffer = nullptr;
			return true;
		}
	public:
		//==============================
		// Manage Components
		//==============================
		virtual void* CreateComponent(EntityID entityID) override
		{
			KG_ASSERT(i_RegistryAlloc);
			KG_ASSERT(m_CompMetadata.m_ComponentSize > 0);
			KG_ASSERT(k_MaxEntities > 0);

			// Get the data pointer
			uint8_t* rawComponent
			{ 
				&m_ComponentBuffer[entityID * m_CompMetadata.m_ComponentSize] 
			};

			// Return existing component if it already exists
			bool hasComponent{ m_ValidEntityArray[entityID] };
			if (hasComponent)
			{
				return (void*)rawComponent;
			}

			// Update array metadata
			m_ValidEntityArray[entityID] = true;
			m_ComponentCount++;

			return (void*)rawComponent;
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
			return (void*)&m_ComponentBuffer[entityID * m_CompMetadata.m_ComponentSize];
		}

		std::span<bool> GetValidEntityArray()
		{
			return m_ValidEntityArray;
		}

		//==============================
		// Query State
		//==============================
		virtual bool HasComponent(EntityID entityID) override
		{
			KG_ASSERT(entityID < (EntityID)m_ValidEntityArray.size());

			return m_ValidEntityArray[entityID];
		}

		virtual ComponentCount GetComponentCount() override
		{
			return m_ComponentCount;
		}

		//==============================
		// Getters/Setters
		//==============================
		virtual void SetComponentMetadata(const ComponentMetadata& metadata)
		{
			m_CompMetadata = metadata;
		}
		virtual const ComponentMetadata& GetComponentMetadata() const
		{
			return m_CompMetadata;
		}
	private:
		//==============================
		// Internal Fields
		//==============================
		// Data array(s)
		uint8_t* m_ComponentBuffer{ nullptr };
		std::array<bool, k_MaxEntities> m_ValidEntityArray{};
		ComponentCount m_ComponentCount{ 0 };

		// Component info
		ComponentMetadata m_CompMetadata{};

		//==============================
		// Injected Section
		//==============================
		EntityRegistry* i_EntityRegistry{ nullptr };
		Memory::IAllocator* i_RegistryAlloc{ nullptr };
	private:
		//==============================
		// Owning Class(s)
		//==============================
		friend class RegistryInternal;
		friend class ComponentRegistry;
	};
}

