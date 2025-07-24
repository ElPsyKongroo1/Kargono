#pragma once

#include "Modules/ECSTest/ECSComponentArrayTest.h"

#include "Kargono/Core/Base.h"
#include "Kargono/Memory/IAllocator.h"

#include "Modules/ECSTest/ECSEntityRegistryTest.h"

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
		[[nodiscard]] bool Init(Memory::IAllocator* parentAlloc, EntityRegistry* registry)
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
			if (m_ComponentTypes.contains(typeName))
			{
				return false;
			}

			// Add the component type and its array
			m_ComponentTypes.insert({typeName, m_NextComponentType});

			PackedArray<t_Component>* newArray = i_RegistryAlloc->Alloc<PackedArray<t_Component>>();
			if (!newArray)
			{
				return false;
			}

			newArray->Init(i_EntityRegistry);
			m_ComponentArrays.insert({typeName, newArray});

			// Increment the value so that the next component registered will be different
			++m_NextComponentType;
			return true;
		}

		template<typename t_Component>
		[[nodiscard]] bool AddComponent(EntityID entityID, t_Component component)
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
		Expected<ComponentType> GetComponentType()
		{
			const char* typeName = typeid(t_Component).name();

			// Check if component type is registered
			if (!m_ComponentTypes.contains(typeName))
			{
				return {};
			}

			return m_ComponentTypes[typeName];
		}

		template<typename t_Component>
		IComponentArray* GetComponentArray()
		{
			const char* typeName = typeid(t_Component).name();

			if (!m_ComponentTypes.contains(typeName))
			{
				return nullptr;
			}

			return m_ComponentArrays[typeName];
		}

		//==============================
		// Pseudo Events
		//==============================
		void DestroyEntity(EntityID entityID)
		{
			Signature signature = i_EntityRegistry->GetSignature(entityID);

			// Handle all component arrays
			for (const auto& [componentName, array] : m_ComponentArrays)
			{
				KG_ASSERT(m_ComponentTypes.contains(componentName));

				ComponentType currentType = m_ComponentTypes[componentName];

				if (signature.test(currentType))
				{
					array->RemoveComponent(entityID);
				}
			}
		}
	private:
		//==============================
		// Internal Fields
		//==============================
		// Component data/info
		std::unordered_map<const char*, ComponentType> m_ComponentTypes{};
		std::unordered_map<const char*, IComponentArray*> m_ComponentArrays{};
		// Iterator for adding new components
		ComponentType m_NextComponentType{0};

		//==============================
		// Injected Fields
		//==============================
		Memory::IAllocator* i_RegistryAlloc{ nullptr };
		EntityRegistry* i_EntityRegistry{ nullptr };
	private:
		//==============================
		// Owning Class(s)
		//==============================
		friend class Registry;
	};
}