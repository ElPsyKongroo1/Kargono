#pragma once

#include "ECSComponentRegistryTest.h"
#include "ECSEntityRegistryTest.h"

#include "Kargono/Memory/IAllocator.h"

namespace Kargono::ECS
{
	class Registry
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		Registry() = default;
		~Registry() = default;
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		[[nodiscard]] bool Init(Memory::IAllocator* backingAlloc)
		{
			KG_ASSERT(backingAlloc);

			i_Allocator = backingAlloc;

			if (!m_EntityRegistry.Init())
			{
				return false;
			}

			if (!m_ComponentRegistry.Init(i_Allocator))
			{
				return false;
			}

			return true;
		}

		[[nodiscard]] bool Terminate()
		{
			i_Allocator->Reset();

			return true;
		}

		//==============================
		// Manage Entities
		//==============================
		[[nodiscard]] Expected<EntityID> CreateEntity()
		{
			return m_EntityRegistry.CreateEntity();
		}

		[[nodiscard]] bool DestroyEntity(EntityID entityID)
		{
			if (!m_EntityRegistry.DestroyEntity(entityID))
			{
				return false;
			}

			m_ComponentRegistry.EntityDestroyed(entityID);
		}

		//==============================
		// Manage Components
		//==============================
		template<typename t_Component>
		[[nodiscard]] bool RegisterComponent()
		{
			return m_ComponentRegistry.RegisterComponent<t_Component>();
		}

		template<typename t_Component>
		[[nodiscard]] bool AddComponent(EntityID entityID, t_Component component)
		{
			// Add the component to the entity in the component registry
			if (!m_ComponentRegistry.AddComponent<t_Component>(entityID, component))
			{
				return false;
			}

			// Update the signature of the entity
			Expected<Signature> entitySignature = m_EntityRegistry.GetSignature(entity);
			if (!entitySignature)
			{
				return false;
			}
			entitySignature->set(m_ComponentRegistry.GetComponentType<t_Component>(), true);
		}

		template<typename t_Component>
		[[nodiscard]] bool RemoveComponent(EntityID entityID)
		{
			// Remove the component from the registry
			if (!m_ComponentRegistry.RemoveComponent<t_Component>(entityID))
			{
				return false;
			}

			// Update the signature of the entity
			Expected<Signature> entitySignature = m_EntityRegistry.GetSignature(entityID);
			if (!entitySignature)
			{
				return false;
			}
			entitySignature->set(m_ComponentRegistry.GetComponentType<t_Component>(), false);
		}

		template<typename t_Component>
		ExpectedRef<t_Component> GetComponent(EntityID entityID)
		{
			return m_ComponentRegistry.GetComponent<t_Component>(entityID);
		}

		template<typename t_Component>
		Expected<ComponentType> GetComponentType()
		{
			return m_ComponentRegistry.GetComponentType<t_Component>();
		}

	private:
		//==============================
		// Internal Fields
		//==============================
		EntityRegistry m_EntityRegistry;
		ComponentRegistry m_ComponentRegistry;

		//==============================
		// Injected Dependencies
		//==============================
		// Allocator for all pools
		Memory::IAllocator* i_Allocator{ nullptr };
	};
}