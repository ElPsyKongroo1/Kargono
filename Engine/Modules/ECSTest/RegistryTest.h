#pragma once

#include "Modules/ECSTest/ComponentRegistryTest.h"
#include "Modules/ECSTest/EntityRegistryTest.h"
#include "Modules/ECSTest/Views/IViewTest.h"

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

			if (!m_ComponentRegistry.Init(i_Allocator, &m_EntityRegistry))
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
			if (!m_EntityRegistry.HasEntity(entityID))
			{
				return false;
			}

			m_ComponentRegistry.DestroyEntity(entityID);

			m_EntityRegistry.DestroyEntity(entityID);
			
			return true;
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
		[[nodiscard]] bool AddComponent(EntityID entityID, t_Component& component)
		{
			// Ensure the entity exists in the registry
			if (!m_EntityRegistry.HasEntity(entityID))
			{
				return false;
			}

			// Add the component to the entity in the component registry
			if (!m_ComponentRegistry.AddComponent<t_Component>(entityID, component))
			{
				return false;
			}

			// Get the entity's signature
			Expected<Signature> entitySignature = m_EntityRegistry.GetSignature(entityID);
			KG_ASSERT(entitySignature);

			// Get the relevant component mask/identifier
			Expected<ComponentMask> compMask{ m_ComponentRegistry.GetComponentMask<t_Component>() };
			KG_ASSERT(compMask);

			// Update the signature w/ mask
			entitySignature->SetFlag(compMask.value());
			m_EntityRegistry.SetEntitySignature(entityID, entitySignature.value());

			return true;
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
			KG_ASSERT(entitySignature);

			// Get the relevant component mask/identifier
			Expected<ComponentMask> compMask{ m_ComponentRegistry.GetComponentMask<t_Component>() };
			KG_ASSERT(compMask);

			// Update the signature w/ mask
			entitySignature->ClearFlag(compMask.value());
			m_EntityRegistry.SetEntitySignature(entityID, entitySignature.value());

			return true;
		}

		//==============================
		// Query Registry
		//==============================
		template<typename t_Component>
		ExpectedRef<t_Component> GetComponent(EntityID entityID)
		{
			t_Component* compPtr{(t_Component*)m_ComponentRegistry.GetComponent<t_Component>(entityID)};
			if (!compPtr)
			{
				return {};
			}
			return {*compPtr};
		}
		
		template<typename t_DataType>
		PackedArraysView<t_DataType> GetView()
		{
			return m_ComponentRegistry.GetView<t_DataType>();
		}

		template<typename t_Component>
		Expected<ComponentMask> GetComponentMask()
		{
			return m_ComponentRegistry.GetComponentMask<t_Component>();
		}

		std::span<EntityID> GetAllEntities()
		{
			return m_EntityRegistry.GetAllEntities();
		}

		//==============================
		// Debugging Section // TODO: PLEASE REMOVE
		//==============================
		std::string PrintSignatures()
		{
			return m_EntityRegistry.PrintSignatures();
		}

	private:
		//==============================
		// Internal Fields
		//==============================
		EntityRegistryTest m_EntityRegistry;
		ComponentRegistry m_ComponentRegistry;

		//==============================
		// Injected Dependencies
		//==============================
		// Allocator for all pools
		Memory::IAllocator* i_Allocator{ nullptr };
	};
}