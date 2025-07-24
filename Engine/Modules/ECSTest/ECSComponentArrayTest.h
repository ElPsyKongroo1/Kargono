#pragma once

#include "Modules/ECSTest/ECSCommonTest.h"
#include "Kargono/Memory/IAllocator.h"

#include "Modules/ECSTest/ECSEntityRegistryTest.h"

#include <array>
#include <unordered_map>
#include <cstdint>

namespace Kargono::ECS
{
	class IComponentArray
	{
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		virtual void Init(EntityRegistry* entityRegistry) = 0;
	public:
		//==============================
		// Manage Components
		//==============================
		[[nodiscard]] virtual bool InsertComponent(EntityID entityID, void* component) = 0;
		[[nodiscard]] virtual bool RemoveComponent(EntityID entityID) = 0;
		[[nodiscard]] virtual void* GetComponent(EntityID entityID) = 0;
	};

	template<typename t_Component>
	class BigMatrixArray : public IComponentArray
	{
	private:
		//==============================
		// Constructors/Destructors
		//==============================
		BigMatrixArray() = default;
		~BigMatrixArray() = default;
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		void Init(EntityRegistry* entityRegistry) override
		{
			// Ensure dependencies are valid
			KG_ASSERT(entityRegistry);
			i_EntityRegistry = entityRegistry;
		}
	public:
		//==============================
		// Manage Components
		//==============================
		bool InsertComponent(EntityID entityID, void* component) override
		{
			m_ComponentArray[entityID] = *(t_Component*)component;
			m_ComponentCount++;
		}
		bool RemoveComponent(EntityID entityID) override
		{
			// Clear component data
			memset(&m_ComponentArray[entityID], 0, sizeof(t_Component));
			m_ComponentCount--;
			return true;
		}
		void* GetComponent(EntityID entityID) override
		{
			return m_ComponentArray[entityID];
		}
	private:
		//==============================
		// Internal Fields
		//==============================
		// Packed array and info
		std::array<t_Component, k_MaxEntities> m_ComponentArray{};
		ComponentCount m_ComponentCount{ 0 };

		//==============================
		// Injected Section
		//==============================
		EntityRegistry* i_EntityRegistry{ nullptr };
	private:
		//==============================
		// Owning Class(s)
		//==============================
		friend class Registry;
	};
	
	template<typename t_Component>
	class PackedArray : public IComponentArray
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
		void Init(EntityRegistry* entityRegistry) override
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

			// Insert new component and update maps
			ComponentIndex newComponentID = m_ComponentCount;
			m_EntityToCompIndexMap[entityID] = newComponentID;
			m_CompIndexToEntityMap[newComponentID] = entityID;
			m_ComponentArray[newComponentID] = *(t_Component*)component;
			m_ComponentCount++;

			return true;
		}

		[[nodiscard]] bool RemoveComponent(EntityID entityID) override
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
			
			m_EntityToCompIndexMap.erase(entityID);
			m_CompIndexToEntityMap.erase(indexOfLastComponent);

			m_ComponentCount--;
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

			return &m_ComponentArray[m_EntityToCompIndexMap[entityID]];
		}

	public:
		//==============================
		// Query Entity
		//==============================
		bool HasEntity(EntityID entityID)
		{
			return m_EntityToCompIndexMap.contains(entityID);
		}
	private:
		//==============================
		// Internal Fields
		//==============================
		// Packed array and info
		std::array<t_Component, k_MaxEntities> m_ComponentArray{};
		ComponentCount m_ComponentCount{ 0 };
		// Component <-> entity maps TODO Repalce w/ sparse set
		std::unordered_map<EntityID, ComponentIndex> m_EntityToCompIndexMap{};
		std::unordered_map<ComponentIndex, EntityID> m_CompIndexToEntityMap{};

	private:
		//==============================
		// Injected Section
		//==============================
		EntityRegistry* i_EntityRegistry{ nullptr };
	private:
		//==============================
		// Owning Class(s)
		//==============================
		friend class Registry;
	};
}