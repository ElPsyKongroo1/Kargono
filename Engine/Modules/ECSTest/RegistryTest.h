#pragma once

#include "Modules/ECSTest/ComponentRegistryTest.h"
#include "Modules/ECSTest/EntityRegistryTest.h"
#include "Modules/ECSTest/Views/PackedView.h"
#include "Modules/ECSTest/Views/FlatView.h"

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
			m_ComponentRegistry.Terminate();

			return true;
		}

		[[nodiscard]] bool Clear()
		{
			m_EntityRegistry.Clear();
			m_ComponentRegistry.Clear();

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

		[[nodiscard]] bool RegisterComponent(ComponentIdentifier componentIdentifier,
			size_t componentSize, size_t componentAlignment, 
			ComponentFunctors componentFunctors)
		{
			return m_ComponentRegistry.RegisterComponent(componentIdentifier, 
				componentSize, componentAlignment, componentFunctors);
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

		[[nodiscard]] bool AddComponent(EntityID entityID, ComponentIdentifier identifier ,
			void* component)
		{
			// Ensure the entity exists in the registry
			if (!m_EntityRegistry.HasEntity(entityID))
			{
				return false;
			}

			// Add the component to the entity in the component registry
			if (!m_ComponentRegistry.AddComponent(entityID, identifier, component))
			{
				return false;
			}

			// Get the entity's signature
			Expected<Signature> entitySignature = m_EntityRegistry.GetSignature(entityID);
			KG_ASSERT(entitySignature);

			// Get the relevant component mask/identifier
			Expected<ComponentMask> compMask{ m_ComponentRegistry.GetComponentMask(identifier) };
			KG_ASSERT(compMask);

			// Update the signature w/ mask
			entitySignature->SetFlag(compMask.value());
			m_EntityRegistry.SetEntitySignature(entityID, entitySignature.value());

			return true;
		}

		[[nodiscard]] bool CopyComponents(EntityID srcID, EntityID destID)
		{
			// Ensure the entities exists in the registry
			if (!m_EntityRegistry.HasEntity(srcID) || !m_EntityRegistry.HasEntity(destID))
			{
				return false;
			}

			// Get the entity's signature
			const Signature srcSignature{ m_EntityRegistry.GetSignature(srcID).value() };

			// Loop through each component
			for (ComponentMask mask = 0; mask < sizeof(Signature) * 8; mask++)
			{
				// Check if the component exists
				if (srcSignature.IsFlagSet(mask))
				{
					// Add component to destination
					void* srcComponent = m_ComponentRegistry.GetComponent(srcID, mask);
					KG_ASSERT(srcComponent);
					m_ComponentRegistry.AddOrReplaceComponent(destID, mask, srcComponent);
				}
				else
				{
					// Remove component from destination
					m_ComponentRegistry.RemoveComponent(destID, mask);
				}
			}

			// Update the destination signature
			m_EntityRegistry.SetEntitySignature(destID, srcSignature);

			return true;
		}

		template<typename t_Component, typename... t_Args>
		[[nodiscard]] t_Component& EmplaceComponent(EntityID entityID, t_Args... args)
		{
			KG_ASSERT(m_EntityRegistry.HasEntity(entityID));

			void* component 
			{ 
				m_ComponentRegistry.EmplaceComponent(entityID, std::forward<t_Args>(args)...) 
			};
			KG_ASSERT(component);

			// Get the entity's signature
			Expected<Signature> entitySignature{ m_EntityRegistry.GetSignature(entityID) };
			KG_ASSERT(entitySignature);

			// Get the relevant component mask/identifier
			Expected<ComponentMask> compMask{ m_ComponentRegistry.GetComponentMask<t_Component>() };
			KG_ASSERT(compMask);

			// Update the signature w/ mask
			entitySignature->SetFlag(compMask.value());
			m_EntityRegistry.SetEntitySignature(entityID, entitySignature.value());

			return *(t_Component)component;
		}

		template<typename t_Component, typename... t_Args>
		[[nodiscard]] t_Component& EmplaceOrReplaceComponent(EntityID entityID, t_Args... args)
		{
			KG_ASSERT(m_EntityRegistry.HasEntity(entityID));

			void* component
			{
				m_ComponentRegistry.EmplaceOrReplaceComponent(entityID, 
					std::forward<t_Args>(args)...)
			};
			KG_ASSERT(component);

			// Get the entity's signature
			Expected<Signature> entitySignature{ m_EntityRegistry.GetSignature(entityID) };
			KG_ASSERT(entitySignature);

			// Get the relevant component mask/identifier
			Expected<ComponentMask> compMask{ m_ComponentRegistry.GetComponentMask<t_Component>() };
			KG_ASSERT(compMask);

			// Update the signature w/ mask
			entitySignature->SetFlag(compMask.value());
			m_EntityRegistry.SetEntitySignature(entityID, entitySignature.value());

			return *(t_Component)component;
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

		[[nodiscard]] bool RemoveComponent(EntityID entityID, ComponentIdentifier identifier)
		{
			// Remove the component from the registry
			if (!m_ComponentRegistry.RemoveComponent(entityID, identifier))
			{
				return false;
			}

			// Update the signature of the entity
			Expected<Signature> entitySignature = m_EntityRegistry.GetSignature(entityID);
			KG_ASSERT(entitySignature);

			// Get the relevant component mask/identifier
			Expected<ComponentMask> compMask{ m_ComponentRegistry.GetComponentMask(identifier) };
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

		void* GetComponent(EntityID entityID, ComponentIdentifier identifier)
		{
			void* compPtr{ m_ComponentRegistry.GetComponent(entityID, identifier) };
			if (!compPtr)
			{
				return nullptr;
			}
			return compPtr;
		}
		
		template<typename... t_ComponentTypes>
		PackedView<sizeof...(t_ComponentTypes)> GetPackedView()
		{
			constexpr size_t k_NumComponents{ sizeof...(t_ComponentTypes) };
			if constexpr (k_NumComponents == 1)
			{
				// Get component identifier
				constexpr auto identifierStr{ GetUniqueIdentifier<t_ComponentTypes...>() };
				constexpr ComponentIdentifier identifier =
					Utility::FileSystem::CRCFromString(identifierStr.CString());

				return m_ComponentRegistry.GetSinglePackedView(identifier);
			}
			else
			{
				// Fill array w/ templated type identifiers
				constexpr ComponentIDList<k_NumComponents> identifiers
				{
					Utility::FileSystem::CRCFromString(GetUniqueIdentifier<t_ComponentTypes>().CString())...
				};

				// Get the multi packed view
				return m_ComponentRegistry.GetMultiPackedView(identifiers);
			}
		}

		template<size_t t_NumComponents>
		PackedView<t_NumComponents> GetPackedView(
			const ComponentIDList<t_NumComponents>& identifiers)
		{
			if constexpr (t_NumComponents == 1)
			{
				return m_ComponentRegistry.GetSinglePackedView(identifiers[0]);
			}
			else
			{
				return m_ComponentRegistry.GetMultiPackedView<t_NumComponents>(identifiers);
			}
		}

		template<typename... t_ComponentTypes>
		FlatView<sizeof...(t_ComponentTypes)> GetFlatView()
		{
			constexpr size_t k_NumComponents{ sizeof...(t_ComponentTypes) };
			if constexpr (k_NumComponents == 1)
			{
				// Get component identifier
				constexpr auto identifierStr{ GetUniqueIdentifier<t_ComponentTypes...>() };
				constexpr ComponentIdentifier identifier =
					Utility::FileSystem::CRCFromString(identifierStr.CString());

				return m_ComponentRegistry.GetSingleFlatView(identifier);
			}
			else
			{
				// Fill array w/ templated type identifiers
				constexpr ComponentIDList<k_NumComponents> identifiers
				{
					Utility::FileSystem::CRCFromString(GetUniqueIdentifier<t_ComponentTypes>().CString())...
				};

				// Get the multi flat view
				return m_ComponentRegistry.GetMultiFlatView(identifiers);
			}
		}

		template<size_t t_NumComponents>
		FlatView<t_NumComponents> GetFlatView(
			const ComponentIDList<t_NumComponents>& identifiers)
		{
			if constexpr (t_NumComponents == 1)
			{
				return m_ComponentRegistry.GetSingleFlatView(identifiers[0]);
			}
			else
			{
				return m_ComponentRegistry.GetMultiFlatView<t_NumComponents>(identifiers);
			}
		}

		template<typename t_Component>
		Expected<ComponentMask> GetComponentMask()
		{
			return m_ComponentRegistry.GetComponentMask<t_Component>();
		}

		Expected<ComponentMask> GetComponentMask(ComponentIdentifier identifier)
		{
			return m_ComponentRegistry.GetComponentMask(identifier);
		}

		std::span<EntityID> GetAllEntities()
		{
			return m_EntityRegistry.GetAllEntities();
		}

		template<typename t_ComponentType>
		bool HasComponent(EntityID entityID)
		{
			// Get component identifier
			constexpr auto identifierStr{ GetUniqueIdentifier<t_ComponentType>() };
			constexpr ComponentIdentifier identifier =
				Utility::FileSystem::CRCFromString(identifierStr.CString());

			return HasComponent(entityID, identifier);
		}

		bool HasComponent(EntityID entityID, ComponentIdentifier identifier)
		{
			if (!IsComponentRegistered(identifier))
			{
				return false;
			}

			return m_ComponentRegistry.HasComponent(entityID, identifier);
		}

		template<typename t_ComponentType>
		bool IsComponentRegistered()
		{
			// Get component identifier
			constexpr auto identifierStr{ GetUniqueIdentifier<t_ComponentType>() };
			constexpr ComponentIdentifier identifier =
				Utility::FileSystem::CRCFromString(identifierStr.CString());

			// Check if the component is registered
			return m_ComponentRegistry.IsComponentRegistered(identifier);
		}

		bool IsComponentRegistered(ComponentIdentifier identifier)
		{
			return m_ComponentRegistry.IsComponentRegistered(identifier);
		}

		//==============================
		// Interact w/ Other Registries
		//==============================
		void DuplicateRegistry(Registry& otherRegistry)
		{
			// Clear the other registry

			// Copy over all entities

			// Copy over all components

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