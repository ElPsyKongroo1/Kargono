#pragma once

#include "Modules/ECSTest/ComponentArrays/IComponentStoreTest.h"

namespace Kargono::ECS
{
	template<typename t_Component>
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
		EntityRegistryTest* i_EntityRegistry{ nullptr };
	private:
		//==============================
		// Owning Class(s)
		//==============================
		friend class Registry;
	};
}

