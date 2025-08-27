#pragma once

#include "Modules/ECSTest/ComponentArrays/IComponentStoreTest.h"
#include "Modules/ECSTest/Views/PackedView.h"

#include "Modules/ECSTest/DataStructures/SparseSetTest.h"

#include "Kargono/Utility/Operations.h"

namespace Kargono::ECS
{
	class PackedArray : public IComponentStore
	{
	private:
		//==============================
		// Constructors/Destructors
		//==============================
		PackedArray() = default;
		~PackedArray() = default;
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		void Init(EntityRegistryTest* entityRegistry, Memory::IAllocator* regAlloc, 
			ComponentMetadata metadata) override
		{
			// Ensure dependencies are valid
			KG_ASSERT(entityRegistry);
			KG_ASSERT(regAlloc);
			i_EntityRegistry = entityRegistry;
			i_RegistryAlloc = regAlloc;
			m_CompMetadata = metadata;

			AllocateBuffer(m_CompMetadata.m_ComponentSize, m_CompMetadata.m_ComponentAlignment);
		}

		void Terminate() override
		{
			if (m_ComponentBuffer)
			{
				DeallocateBuffer(
					m_CompMetadata.m_ComponentSize, m_CompMetadata.m_ComponentAlignment);
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

			bool success{ i_RegistryAlloc->DeallocRaw(m_ComponentBuffer, componentAlignment) };

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
		virtual void* CreateComponent(EntityID entityID) override
		{
			// Check if component already exists
			if (HasComponent(entityID))
			{
				return nullptr;
			}

			// Insert entity into sparse set
			ComponentIndex compIndex{ m_EntityComponentSet.InsertElement(entityID) };
			if (compIndex == m_EntityComponentSet.k_InvalidDenseIndex)
			{
				return nullptr;
			}

			uint8_t* rawComponent{ &m_ComponentBuffer[compIndex * m_ComponentSize] };

			return (void*)rawComponent;
		}

		[[nodiscard]] bool RemoveComponent(EntityID entityID) override
		{
			// Check if component does not exist
			if (!HasComponent(entityID))
			{
				return false;
			}

			// Update the sparse list
			ComponentIndex indexOfLastEntity{ m_EntityComponentSet.GetDenseCount() - 1};
			ComponentIndex indexOfRemovedEntity{ m_EntityComponentSet.DeleteElement(entityID) };

			// Check for failure to remove entity from sparse set
			KG_ASSERT(m_EntityComponentSet.IsValidDenseIndex(indexOfRemovedEntity));

			// Update the component list
			uint8_t* destination{ &m_ComponentBuffer[(size_t)indexOfRemovedEntity * m_ComponentSize] };
			uint8_t* source{ &m_ComponentBuffer[(size_t)indexOfLastEntity * m_ComponentSize] };
			memcpy(destination, source, m_ComponentSize);

			return true;
		}

	public:
		//==============================
		// Query Component Array
		//==============================
		void* GetComponent(EntityID entityID)
		{
			if (!HasComponent(entityID))
			{
				return nullptr;
			}

			return &m_ComponentBuffer
			[
				(size_t)m_EntityComponentSet.GetDenseIndex(entityID) * m_ComponentSize
			];
		}

		std::span<EntityID> GetEntityList()
		{
			return m_EntityComponentSet.GetDenseList();
		}

		PackedSparseSet* GetSparseSet()
		{
			return &m_EntityComponentSet;
		}

	public:
		//==============================
		// Query State
		//==============================
		virtual bool HasComponent(EntityID entityID) override
		{
			return m_EntityComponentSet.HasSparseIndex(entityID);
		}

		//==============================
		// Getters/Setters
		//==============================
		virtual void SetComponentFunctors(const ComponentFunctors& functors) override
		{
			m_ComponentFunctors = functors;
		}
		virtual const ComponentFunctors& GetComponentFunctors() const
		{
			return m_ComponentFunctors;
		}
	private:
		//==============================
		// Internal Fields
		//==============================
		
		// Packed array data
		uint8_t* m_ComponentBuffer{ nullptr };
		PackedSparseSet m_EntityComponentSet{ k_MaxEntities, k_MaxEntities };

		// Component metadata
		ComponentMetadata m_CompMetadata{};
	private:
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

