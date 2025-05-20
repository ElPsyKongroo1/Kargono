#pragma once

#include "ECSComponentArrayTest.h"
#include "Kargono/Core/Base.h"

#include <unordered_map>

#include <memory>

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

			std::shared_ptr<ComponentArray<t_Component>> newArray 
			{
				std::make_shared<ComponentArray<t_Component>>()
			};

			m_ComponentArrays.insert({typeName, newArray});

			// Increment the value so that the next component registered will be different
			++m_NextComponentType;
			return true;
		}

		template<typename t_Component>
		[[nodiscard]] bool AddComponent(EntityID entityID, t_Component component)
		{
			return GetComponentArray<t_Component>()->InsertComponent(entityID, component);
		}

		template<typename t_Component>
		[[nodiscard]] bool RemoveComponent(EntityID entityID)
		{
			return GetComponentArray<t_Component>()->RemoveComponent(entityID);
		}

		template<typename t_Component>
		ExpectedRef<t_Component> GetComponent(EntityID entityID)
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
		std::shared_ptr<ComponentArray<t_Component>> GetComponentArray()
		{
			const char* typeName = typeid(t_Component).name();

			if (!m_ComponentTypes.contains(typeName))
			{
				return nullptr;
			}

			return std::static_pointer_cast<ComponentArray<t_Component>>(m_ComponentArrays[typeName]);
		}

		//==============================
		// Pseudo Events
		//==============================
		void EntityDestroyed(EntityID entityID)
		{
			// Notify each component array that an entity has been destroyed
			// If it has a component for that entity, it will remove it
			for (const auto& [componentName, array] : m_ComponentArrays)
			{
				array->EntityDestroyed(entityID);
			}
		}
	private:
		//==============================
		// Internal Fields
		//==============================
		// Component data/info
		std::unordered_map<const char*, ComponentType> m_ComponentTypes{};
		std::unordered_map<const char*, std::shared_ptr<IComponentArray>> m_ComponentArrays{};
		// Iterator for adding new components
		ComponentType m_NextComponentType{0};

	private:
		//==============================
		// Owning Class(s)
		//==============================
		friend class Registry;
	};
}