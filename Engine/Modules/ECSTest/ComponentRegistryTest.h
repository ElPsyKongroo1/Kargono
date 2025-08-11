#pragma once

#include "Modules/ECSTest/ComponentArrays/IComponentStoreTest.h"
#include "Modules/ECSTest/ComponentArrays/PackedArray.h"
#include "Modules/ECSTest/ComponentArrays/FlatArray.h"

#include "Modules/ECSTest/Views/PackedView.h"
#include "Modules/ECSTest/Views/FlatView.h"

#include "Kargono/Core/Base.h"
#include "Kargono/Memory/IAllocator.h"

#include "Modules/ECSTest/EntityRegistryTest.h"

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
	public:
		//==============================
		// Manage Component(s)
		//==============================
		template<typename t_Component>
		[[nodiscard]] bool RegisterComponent()
		{
			const char* typeName = typeid(t_Component).name();

			// Check if component already exists
			if (m_ComponentMasks.contains(typeName))
			{
				return false;
			}

			// Add the component type and its array
			m_ComponentMasks.insert({typeName, m_NextComponentType});

#if 1 // Default to packed arrays
			// Create packed array using the provided allocator
			uint8_t* componentBuffer = i_RegistryAlloc->AllocRaw(sizeof(PackedArray<t_Component>), alignof(PackedArray<t_Component>));
			KG_ASSERT(componentBuffer);

			// Call placement-new to construct array
			PackedArray<t_Component>* newArray = new ((void*)componentBuffer) PackedArray<t_Component>();
#endif

#if 0 // Default to flat arrays
			// Create flat array using the provided allocator
			uint8_t* componentBuffer = i_RegistryAlloc->AllocRaw(sizeof(FlatArray<t_Component>), alignof(FlatArray<t_Component>));
			KG_ASSERT(componentBuffer);

			// Call placement-new to construct array
			FlatArray<t_Component>* newArray = new ((void*)componentBuffer) FlatArray<t_Component>();
#endif

			KG_ASSERT(newArray);

			newArray->Init(i_EntityRegistry);
			m_ComponentArrays.insert({typeName, newArray});

			// Increment the value so that the next component registered will be different
			m_NextComponentType++;
			KG_ASSERT(m_NextComponentType < sizeof(m_NextComponentType) * 8);

			return true;

		}

		template<typename t_Component>
		[[nodiscard]] bool AddComponent(EntityID entityID, t_Component& component)
		{
			return GetComponentArray<t_Component>()->InsertComponent(entityID, &component);
		}

		template<typename t_Component>
		[[nodiscard]] bool RemoveComponent(EntityID entityID)
		{
			return GetComponentArray<t_Component>()->RemoveComponent(entityID);
		}

		template<typename t_Component>
		void* GetComponent(EntityID entityID)
		{
			return GetComponentArray<t_Component>()->GetComponent(entityID);
		}

		//==============================
		// Query Components
		//==============================
		template<typename t_Component>
		Expected<ComponentMask> GetComponentMask()
		{
			const char* typeName = typeid(t_Component).name();

			// Check if component type is registered
			if (!m_ComponentMasks.contains(typeName))
			{
				return {};
			}

			return m_ComponentMasks[typeName];
		}

		template<typename t_Component>
		IComponentStore* GetComponentArray()
		{
			const char* typeName = typeid(t_Component).name();

			if (!m_ComponentMasks.contains(typeName))
			{
				bool success = RegisterComponent<t_Component>();
				KG_ASSERT(success);
			}

			return m_ComponentArrays[typeName];
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
			PackedArray<t_ComponentType>* packedStore{ (PackedArray<t_ComponentType>*)compStore };
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
			PackedArray<t_ComponentType>* packedStore{ (PackedArray<t_ComponentType>*)compStore };
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
		template<typename t_DataType>
		FlatView GetFlatView()
		{
			// Check if the component array exists
			Expected<ComponentMask> compMask{ GetComponentMask<t_DataType>() };

			if (!compMask)
			{
				return {};
			}

			// Construct & return the view
			IComponentStore* compStore{ GetComponentArray<t_DataType>() };
			KG_ASSERT(compStore);

			//TODO: Fix this please!!! This needs to use the IView interface instead

			// Convert to packed array 
			FlatArray<t_DataType>* flatStore{ (FlatArray<t_DataType>*)compStore };
			return flatStore->GetFlatView();
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
		std::unordered_map<const char*, ComponentMask> m_ComponentMasks{};
		std::unordered_map<const char*, IComponentStore*> m_ComponentArrays{};
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