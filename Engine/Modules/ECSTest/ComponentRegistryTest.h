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
#if 1 // Packed Array
				i_RegistryAlloc->DeallocRaw((uint8_t*)componentStore, alignof(PackedArray));
#endif
#if 0 // Flat Array
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

#if 1 // Default to packed arrays
			// Create packed array using the provided allocator
			uint8_t* componentBuffer = i_RegistryAlloc->AllocRaw(sizeof(PackedArray), alignof(PackedArray));
			KG_ASSERT(componentBuffer);

			// Call placement-new to construct array
			PackedArray* newArray = new ((void*)componentBuffer) PackedArray();
#endif

#if 0 // Default to flat arrays
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

		template<typename t_Component>
		IComponentStore* GetComponentArray()
		{
			constexpr auto uniqueName{ GetUniqueIdentifier<t_Component>() };
			constexpr ComponentIdentifier identifier =
				Utility::FileSystem::CRCFromString(uniqueName.CString());

			if (!m_ComponentMasks.contains(identifier))
			{
				bool success = RegisterComponent<t_Component>();
				KG_ASSERT(success);
			}


			KG_ASSERT(m_ComponentArrays[identifier]);
			return m_ComponentArrays[identifier];
		}

		IComponentStore* GetComponentArray(ComponentIdentifier identifier)
		{
			KG_ASSERT(m_ComponentMasks.contains(identifier));
			KG_ASSERT(m_ComponentArrays[identifier]);

			return m_ComponentArrays[identifier];
		}

		template<typename t_ComponentType>
		PackedView<t_ComponentType> GetSinglePackedView()
		{
			// Check if the component array exists
			Expected<ComponentMask> compMask{ GetComponentMask<t_ComponentType>() };

			if (!compMask)
			{
				return {};
			}

			// Construct & return the view
			IComponentStore* compStore{ GetComponentArray<t_ComponentType>() };
			KG_ASSERT(compStore);

			// TODO: Fix this please!!! This needs to use the IView interface instead

			// Convert to packed array 
			PackedArray* packedStore{ (PackedArray*)compStore };
			return PackedView<t_ComponentType>{ packedStore->GetEntityList() };
		}

		template<typename... t_ComponentTypes>
		PackedView<t_ComponentTypes...> GetMultiPackedView()
		{
			constexpr size_t k_NumComponents{ sizeof...(t_ComponentTypes)};

			// Create return array
			std::array<PackedSparseSet*, k_NumComponents> returnArray{};

			// Fill array via parameter pack expansion
			FillPackedViewData<t_ComponentTypes...>(returnArray);

			// Create packed view from array
			return PackedView<t_ComponentTypes...>(returnArray);
		}

	private:
		template<typename t_ComponentType>
		PackedSparseSet* RetrieveSparseSet()
		{
			Expected<ComponentMask> compMask = GetComponentMask<t_ComponentType>();
			if (!compMask)
			{
				return nullptr;
			}

			IComponentStore* compStore = GetComponentArray<t_ComponentType>();
			KG_ASSERT(compStore);

			// Convert to packed array 
			PackedArray* packedStore{ (PackedArray*)compStore };
			return packedStore->GetSparseSet();
		}

		// Comparison helper(s)
		template<typename... t_ComponentTypes>
		bool FillPackedViewData(std::array<PackedSparseSet*, sizeof...(t_ComponentTypes)>& dataArray)
		{
			bool allValid = true;

			// Generate index sequence for the number of components
			[&] <std::size_t... t_IndexSeq> (std::index_sequence<t_IndexSeq...>)
			{
				// Fold expression to fill each index of the array
				(([&]
					{
						PackedSparseSet* set{ RetrieveSparseSet<t_ComponentTypes>() };
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
			}(std::index_sequence_for<t_ComponentTypes...>{});

			return allValid;
		}

	public:
		template<typename t_ComponentType>
		FlatView<t_ComponentType> GetSingleFlatView()
		{
			// Check if the component array exists
			Expected<ComponentMask> compMask{ GetComponentMask<t_ComponentType>() };

			if (!compMask)
			{
				return {};
			}

			// Construct & return the view
			IComponentStore* compStore{ GetComponentArray<t_ComponentType>() };
			KG_ASSERT(compStore);

			// TODO: Fix this please!!! This needs to use the IView interface instead

			// Convert to Flat array 
			FlatArray* FlatStore{ (FlatArray*)compStore };
			return FlatView<t_ComponentType>{ FlatStore->GetValidEntityArray() };
		}

		template<typename... t_ComponentTypes>
		FlatView<t_ComponentTypes...> GetMultiFlatView()
		{
			constexpr size_t k_NumComponents{ sizeof...(t_ComponentTypes) };

			// Create return array
			std::array<std::span<bool>, k_NumComponents> returnArray{};

			// Fill array via parameter pack expansion
			FillFlatViewData<t_ComponentTypes...>(returnArray);

			// Create Flat view from array
			return FlatView<t_ComponentTypes...>(returnArray);
		}

	private:
		template<typename... t_ComponentTypes>
		bool FillFlatViewData(std::array<std::span<bool>, sizeof...(t_ComponentTypes)>& dataArray)
		{
			bool allValid = true;

			std::size_t index{ 0 };
			([&] 
			{
				Expected<ComponentMask> compMask = GetComponentMask<t_ComponentTypes>();
				if (!compMask)
				{
					allValid = false;
				}

				IComponentStore* compStore = GetComponentArray<t_ComponentTypes>();
				KG_ASSERT(compStore);

				// Convert to Flat array 
				FlatArray* flatStore{ (FlatArray*)compStore };

				// Store the 
				dataArray[index] = flatStore->GetValidEntityArray();
				index++;
			}(), ...);

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