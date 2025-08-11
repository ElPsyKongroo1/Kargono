#pragma once

#include "Modules/ECSTest/ComponentArrays/IComponentStoreTest.h"
#include "Modules/ECSTest/Views/PackedView.h"

#include "Modules/ECSTest/DataStructures/SparseSetTest.h"

#include "Kargono/Utility/Operations.h"

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
			KG_ASSERT(m_EntityComponentSet.IsValidDenseIndex(indexOfRemovedEntity));
			KG_ASSERT(indexOfRemovedEntity < m_ComponentArray.size());

			// Update the component list
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
		// Query Entity
		//==============================
		bool HasEntity(EntityID entityID)
		{
			return m_EntityComponentSet.HasSparseIndex(entityID);
		}

	private:
		//==============================
		// Debugging Functions
		//==============================
		void PrintSparseSet()
		{
			KG_TRACE_INFO(m_EntityComponentSet.Print());
		}
		void PrintComponentArray()
		{
			EntityID denseCount = (EntityID)m_EntityComponentSet.GetDenseCount();
			
			KG_ASSERT(denseCount < m_ComponentArray.size());
			std::stringstream ss;
			ss << "Component Array (Does Value Exist?): ";
			for (size_t i = 0; i < denseCount; i++)
			{
				uint8_t* compPtr = (uint8_t*)&m_ComponentArray[i];
				bool isZero = Utility::Operations::IsBufferZero(compPtr, sizeof(t_Component));
				ss << isZero << ' ';
			}

			KG_TRACE_INFO(ss.str());
		}
	private:
		//==============================
		// Internal Fields
		//==============================
		// Packed array and info
		std::array<t_Component, k_MaxEntities> m_ComponentArray{};
		// EntityID <-> ComponentIndex data structure
		PackedSparseSet m_EntityComponentSet{ k_MaxEntities, k_MaxEntities };
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
		friend class ComponentRegistry;
	};
}

