#pragma once

#include "ECSCommonTest.h"

#include <array>
#include <unordered_map>
#include <cstdint>

namespace Kargono::ECS
{
	class IComponentArray
	{
	public:
		//==============================
		// Manage Entity
		//==============================
		virtual void EntityDestroyed(EntityID entityID) = 0;
	};

	template<typename t_Component>
	class ComponentArray : public IComponentArray
	{
	private:
		//==============================
		// Constructors/Destructors
		//==============================
		ComponentArray() = default;
		~ComponentArray() = default;
	public:
		//==============================
		// Manage Components
		//==============================
		[[nodiscard]] bool InsertComponent(EntityID entityID, t_Component component)
		{
			// Check if component already exists
			if (m_EntityToCompIndexMap.contains(entityID))
			{
				return false;
			}

			// Insert new component and update maps
			ComponentIndex newComponentID = m_ComponentCount;
			m_EntityToCompIndexMap[entityID] = newComponentID;
			m_CompIndexToEntityMap[newComponentID] = entityID;
			m_ComponentArray[newComponentID] = component;
			m_ComponentCount++;

			return true;
		}

		[[nodiscard]] bool RemoveComponent(EntityID entityID)
		{
			// Check if component does not exist
			if (!m_EntityToCompIndexMap.contains(entityID))
			{
				return false;
			}

			// Move last component into the deleted entity's component location
			ComponentIndex indexOfRemovedEntity = m_EntityToCompIndexMap[entityID];
			ComponentIndex indexOfLastComponent = m_ComponentCount - 1;
			m_ComponentArray[indexOfRemovedEntity] = m_ComponentArray[indexOfLastComponent];

			// Update map to point entity to last element
			EntityID entityOfLastComponent = m_CompIndexToEntityMap[indexOfLastComponent];
			m_EntityToCompIndexMap[entityOfLastComponent] = indexOfRemovedEntity;
			m_CompIndexToEntityMap[indexOfRemovedEntity] = entityOfLastComponent;
			
			m_EntityToCompIndexMap.erase(entity);
			m_CompIndexToEntityMap.erase(indexOfLastComponent);

			m_ComponentCount--;
			return true;
		}

		// TODO: PROBABLY REMOVE THIS
		void EntityDestroyed(EntityID entityID) override
		{
			if (m_EntityToCompIndexMap.contains(entityID))
			{
				// Remove the entity's component if it existed
				RemoveComponent(entityID);
			}
		}

		//==============================
		// Query Component Array
		//==============================
		ExpectedRef<t_Component> GetComponent(EntityID entityID)
		{
			if (!m_EntityToCompIndexMap.contains(entityID))
			{
				return {};
			}

			return m_ComponentArray[m_EntityToCompIndexMap[entityID]];
		}
	private:
		//==============================
		// Internal Fields
		//==============================
		// Packed array and info
		std::array<t_Component, k_MaxEntities> m_ComponentArray{};
		ComponentCount m_ComponentCount{ 0 };
		// Component <-> entity maps
		std::unordered_map<EntityID, ComponentIndex> m_EntityToCompIndexMap{};
		std::unordered_map<ComponentIndex, EntityID> m_CompIndexToEntityMap{};

	private:
		//==============================
		// Owning Class(s)
		//==============================
		friend class Registry;
	};
}