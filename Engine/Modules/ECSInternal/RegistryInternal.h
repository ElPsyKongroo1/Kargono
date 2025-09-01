#pragma once

#include "Modules/ECSInternal/ECSInternalCommon.h"
#include "Modules/ECSInternal/ComponentRegistry.h"
#include "Modules/ECSInternal/EntityRegistry.h"
#include "Modules/ECSInternal/Views/PackedView.h"
#include "Modules/ECSInternal/Views/FlatView.h"
#include "Kargono/Memory/IAllocator.h"

namespace Kargono::ECSInternal
{
	class RegistryInternal
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		RegistryInternal() = default;
		~RegistryInternal() = default;
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
			if (!m_EntityRegistry.Terminate())
			{
				return false;
			}

			if (!m_ComponentRegistry.Terminate())
			{
				return false;
			}

			i_Allocator = nullptr;

			return true;
		}

		[[nodiscard]] bool Clear()
		{
			if (!m_EntityRegistry.Clear())
			{
				return false;
			}
			if (!m_ComponentRegistry.Clear())
			{
				return false;
			}

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
			ComponentMetadata metadata)
		{
			return m_ComponentRegistry.RegisterComponent(componentIdentifier, 
				metadata);
		}

		[[nodiscard]] bool ClearComponentStore(ComponentIdentifier identifier)
		{
#if 0
			PackedView<1> view{ GetPackedView<1>({identifier}) };
#endif
#if 1
			FlatView<1> view{ GetFlatView<1>({identifier}) };
#endif
			ComponentCount compCount{ m_ComponentRegistry.GetComponentCount(identifier)};

			// Cache relevant entityID's before clearing components
			std::vector<EntityID> cachedIDs;
			cachedIDs.reserve(compCount);
			for (EntityID id : view)
			{
				cachedIDs.emplace_back(id);
			}

			// Clear the components
			m_ComponentRegistry.ClearComponentStore(identifier);

			// Get the relevant component mask
			Expected<ComponentMask> compMask{ m_ComponentRegistry.GetComponentMask(identifier) };
			KG_ASSERT(compMask);

			// Update relevant entity ID signatures
			for (EntityID id : cachedIDs)
			{
				// Update the signature of the entity
				Expected<Signature> entitySignature = m_EntityRegistry.GetSignature(id);
				KG_ASSERT(entitySignature);

				// Update the signature w/ mask
				entitySignature->ClearFlag(compMask.value());
				m_EntityRegistry.SetEntitySignature(id, entitySignature.value());
			}
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

		[[nodiscard]] bool AddComponent(EntityID entityID, ComponentIdentifier identifier,
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

		[[nodiscard]] void* CreateComponent(EntityID entityID, ComponentIdentifier identifier)
		{
			// Ensure the entity exists in the registry
			if (!m_EntityRegistry.HasEntity(entityID))
			{
				return nullptr;
			}

			// Add the component to the entity in the component registry
			void* newComponent{ m_ComponentRegistry.CreateComponent(entityID, identifier) };
			KG_ASSERT(newComponent);

			// Get the entity's signature
			Expected<Signature> entitySignature = m_EntityRegistry.GetSignature(entityID);
			KG_ASSERT(entitySignature);

			// Get the relevant component mask/identifier
			Expected<ComponentMask> compMask{ m_ComponentRegistry.GetComponentMask(identifier) };
			KG_ASSERT(compMask);

			// Update the signature w/ mask
			entitySignature->SetFlag(compMask.value());
			m_EntityRegistry.SetEntitySignature(entityID, entitySignature.value());

			return newComponent;
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
					void* srcComponent = m_ComponentRegistry.GetComponentByMask(srcID, mask);
					KG_ASSERT(srcComponent);
					m_ComponentRegistry.AddOrReplaceComponent(destID, mask, srcComponent);
				}
				else
				{
					// Remove component from destination
					m_ComponentRegistry.RemoveComponentByMask(destID, mask);
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

		ComponentCount GetComponentCount(ComponentIdentifier identifier)
		{
			return m_ComponentRegistry.GetComponentCount(identifier);
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

		bool HasEntity(EntityID entityID)
		{
			return m_EntityRegistry.HasEntity(entityID);
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

	public:
		//==============================
		// Interact w/ Other Registries
		//==============================
		void CopyRegistry(RegistryInternal& otherRegistry)
		{
			// Clear the other registry
			bool success{ otherRegistry.Clear() };
			KG_ASSERT(success);

			// Copy over all entities
			m_EntityRegistry.CopyRegistry(otherRegistry.m_EntityRegistry);

			// Copy over all components
			m_ComponentRegistry.CopyRegistry(otherRegistry.m_ComponentRegistry);
		}

	private:
		//==============================
		// Internal Fields
		//==============================
		// Registries
		EntityRegistry m_EntityRegistry;
		ComponentRegistry m_ComponentRegistry;

	private:
		//==============================
		// Injected Dependencies
		//==============================
		// Allocator for all pools
		Memory::IAllocator* i_Allocator{ nullptr };
	};
}