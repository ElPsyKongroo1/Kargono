#pragma once

#include "Modules/ECSTest/ComponentArrays/IComponentStoreTest.h"

#include "Modules/ECSTest/DataStructures/SparseSetTest.h"

namespace Kargono::ECS
{
	template<typename t_Component>
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
		void Init(EntityRegistryTest* entityRegistry) override
		{
			// Ensure dependencies are valid
			KG_ASSERT(entityRegistry);
			i_EntityRegistry = entityRegistry;
		}
	public:
		//==============================
		// Manage Components
		//==============================
		[[nodiscard]] bool InsertComponent(EntityID entityID, void* component) override
		{
			// Check if component already exists
			if (HasEntity(entityID))
			{
				return false;
			}

			// Insert entity into sparse set
			ComponentIndex compIndex{ m_EntityComponentSet.InsertElement(entityID) };
			if (compIndex == m_EntityComponentSet.k_InvalidDenseIndex)
			{
				return false;
			}

			KG_ASSERT(compIndex < m_ComponentArray.size());

			// Insert component into component array
			m_ComponentArray[compIndex] = *(t_Component*)component;

			return true;
		}

		[[nodiscard]] bool RemoveComponent(EntityID entityID) override
		{
			// Check if component does not exist
			if (!HasEntity(entityID))
			{
				return false;
			}

			// Update the sparse list
			ComponentIndex indexOfLastEntity{ m_EntityComponentSet.GetDenseCount() - 1};
			ComponentIndex indexOfRemovedEntity{ m_EntityComponentSet.DeleteElement(entityID) };

			// Check for failure to remove entity from sparse set
			if (indexOfRemovedEntity == m_EntityComponentSet.k_InvalidDenseIndex)
			{
				return false;
			}

			KG_ASSERT(indexOfRemovedEntity < m_ComponentArray.size());
			m_ComponentArray[indexOfRemovedEntity] = m_ComponentArray[indexOfLastEntity];

			return true;
		}

	public:
		//==============================
		// Query Component Array
		//==============================
		void* GetComponent(EntityID entityID)
		{
			if (!HasEntity(entityID))
			{
				return nullptr;
			}

			return &m_ComponentArray[m_EntityComponentSet.GetDenseIndex(entityID)];
		}

	public:
		//==============================
		// Query Entity
		//==============================
		bool HasEntity(EntityID entityID)
		{
			return m_EntityComponentSet.HasSparseIndex(entityID);
		}
	private:
		//==============================
		// Internal Fields
		//==============================
		// Packed array and info
		std::array<t_Component, k_MaxEntities> m_ComponentArray{};
		// EntityID <-> ComponentIndex data structure
		SparseSet<EntityID, ComponentIndex> m_EntityComponentSet{ k_MaxEntities, k_MaxEntities };
	private:
		//==============================
		// Injected Section
		//==============================
		EntityRegistryTest* i_EntityRegistry{ nullptr };
	private:
		//==============================
		// Owning Class(s)
		//==============================
		friend class Registry;
	};
}

