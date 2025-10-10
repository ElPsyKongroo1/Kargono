#pragma once

#include "Modules/ECSInternal/ComponentArrays/IComponentStore.h"
#include "Modules/ECSInternal/ComponentArrays/PackedArray.h"
#include "Modules/ECSInternal/ComponentArrays/FlatArray.h"
#include "Modules/ECSInternal/Views/PackedView.h"
#include "Modules/ECSInternal/Views/FlatView.h"
#include "Modules/ECSInternal/EntityRegistry.h"
#include "Modules/Assets/AssetService.h"

#include "Kargono/Core/Base.h"
#include "Modules/FileSystem/FileSystem.h"
#include "Kargono/Memory/IAllocator.h"

#include <unordered_map>
#include <memory>

namespace Kargono::ECSInternal
{
	class ComponentRegistry
	{
	private:
		//==============================
		// Constructors/Destructors
		//==============================
		ComponentRegistry() = default;
		~ComponentRegistry() = default;

	public:
		//==============================
		// Lifecycle Functions
		//==============================
		[[nodiscard]] bool Init(Memory::IAllocator* parentAlloc, EntityRegistry* registry)
		{
			KG_ASSERT(parentAlloc);
			KG_ASSERT(registry);
			i_RegistryAlloc = parentAlloc;
			i_EntityRegistry = registry;

			ClearComponentStores();

			m_Active = true;
			return true;
		}

		[[nodiscard]] bool Terminate()
		{
			TerminateComponentStores();

			i_EntityRegistry = nullptr;
			i_RegistryAlloc = nullptr;

			m_Active = false;
			return true;
		}

		[[nodiscard]] bool Clear()
		{
			ClearComponentStores();

			return true;
		}

	private:
		// Helper(s)
		void ClearComponentStores()
		{
			// Clear components from stores
			for (auto [mask, componentStore] : m_ComponentArrays)
			{
				componentStore->Clear();
			}
		}

		void ClearComponentStore(ComponentIdentifier identifier)
		{
			KG_ASSERT(IsComponentRegistered(identifier));

			// Check if the component array exists
			Expected<ComponentMask> compMask{ GetComponentMask(identifier) };
			KG_ASSERT(compMask);

			// Construct & return the view
			IComponentStore* compStore{ GetComponentArray(identifier) };
			KG_ASSERT(compStore);

			compStore->Clear();
		}

		void TerminateComponentStores()
		{
			// Terminate each component store
			for (auto [mask, componentStore] : m_ComponentArrays)
			{
				componentStore->Terminate();
			}

			// Release all component store memory
			for (auto [mask, componentStore] : m_ComponentArrays)
			{
#if 0 // Packed Array
				i_RegistryAlloc->DeallocRaw((uint8_t*)componentStore, alignof(PackedArray));
#endif
#if 1 // Flat Array
				i_RegistryAlloc->DeallocRaw((uint8_t*)componentStore, alignof(FlatArray));
#endif
			}

			// Reset data structures
			m_ComponentArrays.clear();
			m_ComponentMasks.clear();
			m_NextComponentType = 0;
		}
	public:
		//==============================
		// Manage Component(s)
		//==============================
		template<typename t_Component>
		[[nodiscard]] bool RegisterComponent()
		{
			constexpr ComponentIdentifier identifier = GetComponentIdentifier<t_Component>();

			return RegisterComponent(identifier, 
				{ sizeof(t_Component), alignof(t_Component), CreateComponentFunctors<t_Component>() });
		}

		[[nodiscard]] bool RegisterComponent(ComponentIdentifier identifier, 
			ComponentMetadata metadata)
		{
			KG_ASSERT(metadata.m_ComponentSize > 0);
			KG_ASSERT(metadata.m_ComponentAlignment > 0);

			KG_ASSERT(CheckComponentFunctors(metadata.m_CompFunctors));

			// Check if component already exists
			if (m_ComponentMasks.contains(identifier))
			{
				return false;
			}

			// Add the component type and its array
			m_ComponentMasks.insert({ identifier, m_NextComponentType });

#if 0 // Default to packed arrays
			// Create packed array using the provided allocator
			uint8_t* componentBuffer = i_RegistryAlloc->AllocRaw(sizeof(PackedArray), alignof(PackedArray));
			KG_ASSERT(componentBuffer);

			// Call placement-new to construct array
			PackedArray* newArray = new ((void*)componentBuffer) PackedArray();
#endif

#if 1 // Default to flat arrays
			// Create flat array using the provided allocator
			uint8_t* componentBuffer = i_RegistryAlloc->AllocRaw(sizeof(FlatArray), alignof(FlatArray));
			KG_ASSERT(componentBuffer);

			// Call placement-new to construct array
			FlatArray* newArray = new ((void*)componentBuffer) FlatArray();
#endif

			KG_ASSERT(newArray);

			newArray->Init(i_EntityRegistry, i_RegistryAlloc, metadata);
			m_ComponentArrays.insert({ m_NextComponentType, newArray });

			// Increment the value so that the next component registered will be different
			m_NextComponentType++;
			KG_ASSERT(m_NextComponentType < sizeof(m_NextComponentType) * 8);

			return true;

		}

		template<typename t_Component>
		[[nodiscard]] bool AddComponent(EntityID entityID, t_Component& component)
		{
			constexpr ComponentIdentifier identifier = GetComponentIdentifier<t_Component>();

			ComponentMask mask { GetComponentMask(identifier).value() };

			return AddComponentByMask(entityID, mask, (void*)&component);
		}

		[[nodiscard]] bool AddComponent(EntityID entityID, ComponentIdentifier identifier,
			void* component)
		{
			ComponentMask mask{ GetComponentMask(identifier).value() };

			return AddComponentByMask(entityID, mask, component);
		}

		[[nodiscard]] bool AddComponentByMask(EntityID entityID, ComponentMask mask,
			void* component)
		{
			IComponentStore* componentStore{ GetComponentArrayByMask(mask) };
			KG_ASSERT(componentStore);

			// Check if a component already exists
			if (componentStore->HasComponent(entityID))
			{
				return false;
			}

			const ComponentMetadata& metadata{ componentStore->GetComponentMetadata() };
			void* newComponent{ componentStore->CreateComponent(entityID) };
			KG_ASSERT(newComponent);

			CustomComponent* customComp{ nullptr };

			// If this is a custom component, get the custom component data
			if (metadata.m_CustomComponentHandle != Assets::k_EmptyHandle)
			{
				customComp = Assets::AssetService::GetCustomComponent(metadata.m_CustomComponentHandle).get();
				KG_ASSERT(customComp);
			}

			// Copy data over to the new component 
			// TODO: (THIS COULD BE DANGEROUS IF component is incorrect AHHHHH!!!)
			metadata.m_CompFunctors.m_Copy(component,
				newComponent, static_cast<void*>(customComp));

			return true;
		}

		[[nodiscard]] void* CreateComponent(EntityID entityID, ComponentMask mask)
		{
			IComponentStore* componentStore{ GetComponentArrayByMask(mask) };
			KG_ASSERT(componentStore);

			// Check if a component already exists
			if (componentStore->HasComponent(entityID))
			{
				return nullptr;
			}

			const ComponentMetadata& metadata{ componentStore->GetComponentMetadata() };
			void* newComponent{ componentStore->CreateComponent(entityID) };
			KG_ASSERT(newComponent);

			return newComponent;
		}

		[[nodiscard]] bool AddOrReplaceComponent(EntityID entityID, ComponentMask mask,
			void* component)
		{
			IComponentStore* componentStore{ GetComponentArrayByMask(mask) };
			KG_ASSERT(componentStore);

			const ComponentMetadata& metadata{ componentStore->GetComponentMetadata() };
			void* newComponent{ componentStore->CreateComponent(entityID) };
			KG_ASSERT(newComponent);

			CustomComponent* customComp{ nullptr };

			// If this is a custom component, get the custom component data
			if (metadata.m_CustomComponentHandle != Assets::k_EmptyHandle)
			{
				customComp = Assets::AssetService::GetCustomComponent(metadata.m_CustomComponentHandle).get();
				KG_ASSERT(customComp);
			}

			// Copy data over to the new component 
			// TODO: (THIS COULD BE DANGEROUS IF component is incorrect AHHHHH!!!)
			metadata.m_CompFunctors.m_Copy(component, 
				newComponent, static_cast<void*>(customComp));

			return true;
		}

		template<typename t_Component, typename... t_Args>
		[[nodiscard]] void* EmplaceComponent(EntityID entityID, t_Args... args)
		{	
			constexpr ComponentIdentifier identifier = GetComponentIdentifier<t_Component>();

			KG_ASSERT(IsComponentRegistered(identifier));

			// Check if the component array exists
			Expected<ComponentMask> compMask{ GetComponentMask(identifier) };
			KG_ASSERT(compMask);

			// Construct & return the view
			IComponentStore* compStore{ GetComponentArray(identifier) };
			KG_ASSERT(compStore);

			// Simply return existing component if it already exists
			if (compStore->HasComponent(entityID))
			{
				return compStore->GetComponent(entityID);
			}

			// Emplace object on raw data using placement new 
			void* rawComponent{ compStore->CreateComponent(entityID)};
			KG_ASSERT(rawComponent);
			return std::construct_at<t_Component, t_Args...>
				(static_cast<t_Component*>(rawComponent), std::forward<t_Args>(args)...);
		}

		template<typename t_Component, typename... t_Args>
		[[nodiscard]] void* EmplaceOrReplaceComponent(EntityID entityID, t_Args... args)
		{
			constexpr ComponentIdentifier identifier = GetComponentIdentifier<t_Component>();

			KG_ASSERT(IsComponentRegistered(identifier));

			// Check if the component array exists
			Expected<ComponentMask> compMask{ GetComponentMask(identifier) };
			KG_ASSERT(compMask);

			// Construct & return the view
			IComponentStore* compStore{ GetComponentArray(identifier) };
			KG_ASSERT(compStore);

			// Emplace object on raw data using placement new 
			void* rawComponent{ compStore->CreateComponent(entityID) };
			KG_ASSERT(rawComponent);

			return std::construct_at(rawComponent, std::forward<t_Args>(args)...);
		}

		template<typename t_Component>
		[[nodiscard]] bool RemoveComponent(EntityID entityID)
		{
			IComponentStore* componentStore{ GetComponentArray<t_Component>() };
			KG_ASSERT(componentStore);

			return componentStore->RemoveComponent(entityID);
		}

		[[nodiscard]] bool RemoveComponent(EntityID entityID, ComponentIdentifier identifier)
		{
			IComponentStore* componentStore{ GetComponentArray(identifier) };
			KG_ASSERT(componentStore);

			return componentStore->RemoveComponent(entityID);
		}

		[[nodiscard]] bool RemoveComponentByMask(EntityID entityID, ComponentMask mask)
		{
			IComponentStore* componentStore{ GetComponentArrayByMask(mask) };
			KG_ASSERT(componentStore);

			return componentStore->RemoveComponent(entityID);
		}

		template<typename t_Component>
		void* GetComponent(EntityID entityID)
		{
			IComponentStore* componentStore{ GetComponentArray<t_Component>() };
			KG_ASSERT(componentStore);

			return componentStore->GetComponent(entityID);
		}

		void* GetComponent(EntityID entityID, ComponentIdentifier identifier)
		{
			IComponentStore* componentStore{ GetComponentArray(identifier) };
			KG_ASSERT(componentStore);

			return componentStore->GetComponent(entityID);
		}

		void* GetComponentByMask(EntityID entityID, ComponentMask mask)
		{
			IComponentStore* componentStore{ GetComponentArrayByMask(mask) };
			KG_ASSERT(componentStore);

			return componentStore->GetComponent(entityID);
		}
		
		ComponentCount GetComponentCount(ComponentIdentifier identifier)
		{
			KG_ASSERT(IsComponentRegistered(identifier));

			// Get the component store
			Expected<ComponentMask> compMask{ GetComponentMask(identifier) };
			KG_ASSERT(compMask);
			IComponentStore* compStore{ GetComponentArray(identifier) };
			KG_ASSERT(compStore);

			// Get the component count
			return compStore->GetComponentCount();
		}

	private:
		// Helper function(s)
		template<ComponentConcept t_ComponentType>
		constexpr ComponentFunctors CreateComponentFunctors()
		{
			return { TypeErasedCopy<t_ComponentType> };
		}

		bool CheckComponentFunctors(ComponentFunctors functors)
		{
			return static_cast<bool>(functors.m_Copy);
		}

	public:
		//==============================
		// Interact w/ Other Registries
		//==============================
		void CopyRegistry(ComponentRegistry& otherRegistry)
		{
			KG_ASSERT(m_Active);
			KG_ASSERT(otherRegistry.m_Active);

			// Reset the other registry's data
			otherRegistry.TerminateComponentStores();

			// Prepare temporary vector to be sorted by component mask
			std::vector<std::tuple<ComponentMask, ComponentIdentifier, IComponentStore*>> 
				m_SortedCompTypes{};
			m_SortedCompTypes.resize(m_NextComponentType);

			// Fill sorted component types array
			for (auto [componentIdentifier, componentMask] : m_ComponentMasks)
			{
				KG_ASSERT(componentMask < m_NextComponentType);

				// Get the corresponding component store ptr
				IComponentStore* compStore{ m_ComponentArrays[componentMask]};
				KG_ASSERT(compStore);

				m_SortedCompTypes[componentMask] = 
				{ componentMask, componentIdentifier, compStore };
			}
			
			// Loop through sorted array
			for (auto [componentMask, componentIdentifier, compStore] : m_SortedCompTypes)
			{
				// Register all components in other registry
				KG_ASSERT(compStore);
				ComponentMetadata metadata = compStore->GetComponentMetadata();

				CustomComponent* customComp{ nullptr };

				// If this is a custom component, get the custom component data
				if (metadata.m_CustomComponentHandle != Assets::k_EmptyHandle)
				{
					customComp = Assets::AssetService::GetCustomComponent(metadata.m_CustomComponentHandle).get();
					KG_ASSERT(customComp);
				}

				otherRegistry.RegisterComponent(componentIdentifier, metadata);

				// Get a view of all entities in each component store
#if 1 // Flat View
				FlatView<1> view = GetSingleFlatView(componentIdentifier);
#endif
#if 0 // Packed View
				PackedView<1> view = GetSinglePackedView(componentIdentifier);
#endif
				
				IComponentStore* destCompStore
				{
					otherRegistry.GetComponentArray(componentIdentifier)
				};
				KG_ASSERT(destCompStore);

				for (EntityID id : view)
				{
					// Get current component from source
					void* srcComponent{ compStore->GetComponent(id)};
					KG_ASSERT(srcComponent);

					// Create new component at destination
					void* destComponent{ destCompStore->CreateComponent(id) };

					// Copy data over to the new component 
					// TODO: (THIS COULD BE DANGEROUS IF component is incorrect AHHHHH!!!)
					metadata.m_CompFunctors.m_Copy(srcComponent, destComponent,
						static_cast<void*>(customComp));
				}
			}
		}

	public:
		//==============================
		// Query Components
		//==============================
		template<typename t_Component>
		Expected<ComponentMask> GetComponentMask()
		{
			constexpr ComponentIdentifier identifier = GetComponentIdentifier<t_Component>();

			return GetComponentMask(identifier);
		}

		Expected<ComponentMask> GetComponentMask(ComponentIdentifier identifier)
		{
			// Check if component type is registered
			if (!m_ComponentMasks.contains(identifier))
			{
				return {};
			}

			return m_ComponentMasks[identifier];
		}

		template<typename t_Component>
		IComponentStore* GetComponentArray()
		{
			constexpr ComponentIdentifier identifier = GetComponentIdentifier<t_Component>();

			return GetComponentArray(identifier);
		}

		IComponentStore* GetComponentArray(ComponentIdentifier identifier)
		{
			KG_ASSERT(m_ComponentMasks.contains(identifier));
			ComponentMask mask{ m_ComponentMasks.at(identifier)};

			KG_ASSERT(m_ComponentArrays.contains(mask));
			return m_ComponentArrays[mask];
		}

		IComponentStore* GetComponentArrayByMask(ComponentMask mask)
		{
			KG_ASSERT(m_ComponentArrays.contains(mask));
			return m_ComponentArrays[mask];
		}

		PackedView<1> GetSinglePackedView(ComponentIdentifier identifier)
		{
			// Check if the component array exists
			Expected<ComponentMask> compMask{ GetComponentMask(identifier) };

			if (!compMask)
			{
				return {};
			}

			// Construct & return the view
			IComponentStore* compStore{ GetComponentArray(identifier) };
			KG_ASSERT(compStore);

			// Convert to packed array 
			PackedArray* packedStore{ (PackedArray*)compStore };
			return PackedView<1>{ packedStore->GetEntityList() };
		}

		template<size_t t_NumComponents>
		PackedView<t_NumComponents> GetMultiPackedView(
			const ComponentIDList<t_NumComponents>& identifiers)
		{
			// Create return array
			std::array<PackedSparseSet*, t_NumComponents> returnArray{};

			// Fill array via parameter pack expansion
			FillPackedViewData<t_NumComponents>(identifiers, returnArray);

			// Create packed view from array
			return PackedView<t_NumComponents>(returnArray);
		}

	private:
		PackedSparseSet* RetrieveSparseSet(ComponentIdentifier identifier)
		{
			Expected<ComponentMask> compMask{ GetComponentMask(identifier) };
			if (!compMask)
			{
				return nullptr;
			}

			IComponentStore* compStore = GetComponentArray(identifier);
			KG_ASSERT(compStore);

			// Convert to packed array 
			PackedArray* packedStore{ (PackedArray*)compStore };
			return packedStore->GetSparseSet();
		}

		// Comparison helper(s)
		template<size_t t_NumComponents>
		bool FillPackedViewData
		(
			const ComponentIDList<t_NumComponents>& identifiers,
			std::array<PackedSparseSet*, t_NumComponents>& dataArray
		)
		{
			bool allValid = true;

			// Generate index sequence for the number of components
			[&] <std::size_t... t_IndexSeq> (std::index_sequence<t_IndexSeq...>)
			{
				// Fold expression to fill each index of the array
				(([&]
					{
						PackedSparseSet* set{ RetrieveSparseSet(identifiers[t_IndexSeq])};
						if (set)
						{
							// copy into the array
							dataArray[t_IndexSeq] = set;
						}
						else
						{
							allValid = false;
						}
					}()), ...);
			}(std::make_index_sequence<t_NumComponents>{});

			return allValid;
		}

	public:
		FlatView<1> GetSingleFlatView(ComponentIdentifier identifier)
		{
			// Check if the component array exists
			Expected<ComponentMask> compMask{ GetComponentMask(identifier) };

			if (!compMask)
			{
				return {};
			}

			// Construct & return the view
			IComponentStore* compStore{ GetComponentArray(identifier) };
			KG_ASSERT(compStore);

			// Convert to Flat array 
			FlatArray* FlatStore{ (FlatArray*)compStore };
			return FlatView<1>{ FlatStore->GetValidEntityArray() };
		}

		template<size_t t_NumComponents>
		FlatView<t_NumComponents> GetMultiFlatView(
			const ComponentIDList<t_NumComponents>& identifiers)
		{
			// Create return array
			std::array<std::span<bool>, t_NumComponents> returnArray{};

			// Fill array via parameter pack expansion
			FillFlatViewData<t_NumComponents>(identifiers, returnArray);

			// Create Flat view from array
			return FlatView<t_NumComponents>(returnArray);
		}

	private:
		template<size_t t_NumComponents>
		bool FillFlatViewData(
			const ComponentIDList<t_NumComponents>& identifiers,
			std::array<std::span<bool>, t_NumComponents>& dataArray)
		{
			bool allValid = true;

			// Generate index sequence for the number of components
			[&] <std::size_t... t_IndexSeq> (std::index_sequence<t_IndexSeq...>)
			{
				// Fold expression to fill each index of the array
				(([&]
				{
				const ComponentIdentifier& identifier{ identifiers[t_IndexSeq] };

				Expected<ComponentMask> compMask{ GetComponentMask(identifier) };
				if (!compMask)
				{
					allValid = false;
				}

				IComponentStore* compStore{ GetComponentArray(identifier) };
				KG_ASSERT(compStore);

				// Convert to Flat array 
				FlatArray* flatStore{ (FlatArray*)compStore };

				// Store the array
				dataArray[t_IndexSeq] = flatStore->GetValidEntityArray();
				}()), ...);
			}(std::make_index_sequence<t_NumComponents>{});

			return allValid;
		}

		//==============================
		// Pseudo Events
		//==============================
		void DestroyEntity(EntityID entityID)
		{

			Expected<Signature> signature = i_EntityRegistry->GetSignature(entityID);
			KG_ASSERT(signature);

			// Handle all component arrays
			for (const auto& [mask, array] : m_ComponentArrays)
			{
				if (signature->IsFlagSet(mask))
				{
					bool success{ array->RemoveComponent(entityID) };
					KG_ASSERT(success);
				}
			}
		}
	public:
		//==============================
		// Query State
		//==============================
		bool IsComponentRegistered(ComponentIdentifier identifier)
		{
			bool isRegistered{ m_ComponentMasks.contains(identifier) };

			KG_ASSERT(isRegistered ? m_ComponentArrays.contains(m_ComponentMasks[identifier]) : true);
			return isRegistered;
		}

		bool HasComponent(EntityID entityID, ComponentIdentifier identifier)
		{
			Expected<ComponentMask> compMask{ GetComponentMask(identifier) };
			KG_ASSERT(compMask.has_value());

			IComponentStore* compStore{ GetComponentArrayByMask(compMask.value()) };
			KG_ASSERT(compStore);

			return compStore->HasComponent(entityID);
		}

	private:
		//==============================
		// Internal Fields
		//==============================
		// Component data/info
		std::unordered_map<ComponentIdentifier, ComponentMask> m_ComponentMasks{};
		std::unordered_map<ComponentMask, IComponentStore*> m_ComponentArrays{};
		// Iterator for adding new components
		ComponentMask m_NextComponentType{0};
		// Local state
		bool m_Active{ false };

		//==============================
		// Injected Fields
		//==============================
		Memory::IAllocator* i_RegistryAlloc{ nullptr };
		EntityRegistry* i_EntityRegistry{ nullptr };
	private:
		//==============================
		// Owning Class(s)
		//==============================
		friend class RegistryInternal;
	};
}