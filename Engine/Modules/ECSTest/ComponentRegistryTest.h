#pragma once

#include "Modules/ECSTest/ComponentArrays/IComponentStoreTest.h"
#include "Modules/ECSTest/ComponentArrays/PackedArray.h"
#include "Modules/ECSTest/ComponentArrays/FlatArray.h"
#include "Modules/ECSTest/Views/PackedView.h"
#include "Modules/ECSTest/Views/FlatView.h"
#include "Modules/ECSTest/EntityRegistryTest.h"
#include "Modules/Core/Module.h"

#include "Kargono/Core/Base.h"
#include "Kargono/Utility/CompilerInfo.h"
#include "Modules/FileSystem/FileSystem.h"
#include "Kargono/Memory/IAllocator.h"

#include <unordered_map>

namespace Kargono::ECS
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
		[[nodiscard]] bool Init(Memory::IAllocator* parentAlloc, EntityRegistryTest* registry)
		{
			KG_ASSERT(parentAlloc);
			KG_ASSERT(registry);
			i_RegistryAlloc = parentAlloc;
			i_EntityRegistry = registry;

			return true;
		}

		[[nodiscard]] bool Terminate()
		{
			// Terminate each component store
			for (auto [identifier, componentStore] : m_ComponentArrays)
			{
				componentStore->Terminate();
			}

			// Release all component store memory
			for (auto [identifier, componentStore] : m_ComponentArrays)
			{
#if 0 // Packed Array
				i_RegistryAlloc->DeallocRaw((uint8_t*)componentStore, alignof(PackedArray));
#endif
#if 1 // Flat Array
				i_RegistryAlloc->DeallocRaw((uint8_t*)componentStore, alignof(FlatArray));
#endif
			}

			m_ComponentArrays.clear();

			return true;
		}
	public:
		//==============================
		// Manage Component(s)
		//==============================
		template<typename t_Component>
		[[nodiscard]] bool RegisterComponent()
		{
			constexpr auto uniqueName{ GetUniqueIdentifier<t_Component>() };
			constexpr ComponentIdentifier uniqueIdentifier =
				Utility::FileSystem::CRCFromString(uniqueName.CString());

			return RegisterComponent(uniqueIdentifier, sizeof(t_Component), alignof(t_Component));

		}

		[[nodiscard]] bool RegisterComponent(ComponentIdentifier uniqueIdentifier, size_t componentSize, size_t componentAlignment)
		{
			KG_ASSERT(componentSize > 0);
			KG_ASSERT(componentAlignment > 0);

			// Check if component already exists
			if (m_ComponentMasks.contains(uniqueIdentifier))
			{
				return false;
			}

			// Add the component type and its array
			m_ComponentMasks.insert({ uniqueIdentifier, m_NextComponentType });

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

			newArray->Init(i_EntityRegistry, i_RegistryAlloc, componentSize, componentAlignment);
			m_ComponentArrays.insert({ uniqueIdentifier, newArray });

			// Increment the value so that the next component registered will be different
			m_NextComponentType++;
			KG_ASSERT(m_NextComponentType < sizeof(m_NextComponentType) * 8);

			return true;

		}

		template<typename t_Component>
		[[nodiscard]] bool AddComponent(EntityID entityID, t_Component& component)
		{
			IComponentStore* componentStore{ GetComponentArray<t_Component>() };
			KG_ASSERT(componentStore);

			return componentStore->InsertComponent(entityID, &component);
		}

		[[nodiscard]] bool AddComponent(EntityID entityID, ComponentIdentifier identifier,
			void* component)
		{
			IComponentStore* componentStore{ GetComponentArray(identifier) };
			KG_ASSERT(componentStore);

			return componentStore->InsertComponent(entityID, component);
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

	public:
		//==============================
		// Query Components
		//==============================
		template<typename t_Component>
		Expected<ComponentMask> GetComponentMask()
		{
			constexpr auto uniqueName{ GetUniqueIdentifier<t_Component>() };
			constexpr ComponentIdentifier uniqueIdentifier =
				Utility::FileSystem::CRCFromString(uniqueName.CString());

			return GetComponentMask(uniqueIdentifier);
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

		IComponentStore* GetComponentArray(ComponentIdentifier identifier)
		{
			KG_ASSERT(m_ComponentMasks.contains(identifier));
			KG_ASSERT(m_ComponentArrays[identifier]);

			return m_ComponentArrays[identifier];
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
			Expected<ComponentMask> compMask = GetComponentMask(identifier);
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
			for (const auto& [componentName, array] : m_ComponentArrays)
			{
				KG_ASSERT(m_ComponentMasks.contains(componentName));

				ComponentMask currentType = m_ComponentMasks[componentName];

				if (signature->IsFlagSet(currentType))
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

			KG_ASSERT(isRegistered ? m_ComponentArrays.contains(identifier) : true);
			return isRegistered;
		}

	private:
		//==============================
		// Internal Fields
		//==============================
		// Component data/info
		std::unordered_map<ComponentIdentifier, ComponentMask> m_ComponentMasks{};
		std::unordered_map<ComponentIdentifier, IComponentStore*> m_ComponentArrays{};
		// Iterator for adding new components
		ComponentMask m_NextComponentType{0};

		//==============================
		// Injected Fields
		//==============================
		Memory::IAllocator* i_RegistryAlloc{ nullptr };
		EntityRegistryTest* i_EntityRegistry{ nullptr };
	private:
		//==============================
		// Owning Class(s)
		//==============================
		friend class Registry;
	};
}