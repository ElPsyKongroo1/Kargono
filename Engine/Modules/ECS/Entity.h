#pragma once

#include "Kargono/Core/UUID.h"
#include "Modules/Core/Components/IDComponent.h"
#include "Modules/Core/Components/TagComponent.h"
#include "Modules/ECS/Registry.h"
#include "Modules/ECSInternal/RegistryInternal.h"
#include "Modules/Assets/AssetsCommon.h"

namespace Kargono::ECS
{
	struct EntitySerializationContext
	{

	};

	struct EntityDeserializationContext
	{

	};

	class Entity
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		Entity() = default;
		Entity(const Entity& other) = default;
		Entity(ECSInternal::EntityID handle, Registry* registry);
	public:
		//==============================
		// Serialization
		//==============================
		void Serialize(void* context);
		void Deserialize(void* context);
	public:
		//==============================
		// Manage Components
		//==============================
		template<typename t_ComponentType, typename... t_Args>
		t_ComponentType& AddComponent(t_Args&&... args)
		{
			KG_ASSERT(!HasComponent<t_ComponentType>(), "Entity already has component!");
			t_ComponentType& component = 
				m_Registry->m_Registry.EmplaceComponent<t_ComponentType, t_Args...>
				(m_RegistryEntityID, std::forward<t_Args>(args)...);
			return component;
		}

		void AddCustomComponentData(Assets::AssetHandle projectComponentHandle);

		template<typename t_ComponentType, typename... t_Args>
		t_ComponentType& AddOrReplaceComponent(t_Args&&... args)
		{
			t_ComponentType& component
			{ 
				m_Registry->m_Registry.EmplaceOrReplaceComponent<t_ComponentType>(
					m_RegistryEntityID, std::forward<t_Args>(args)...) 
			};
			return component;
		}

		template<typename t_ComponentType>
		void RemoveComponent()
		{
			KG_ASSERT(HasComponent<t_ComponentType>(), "Entity does not have the component!");
			m_Registry->m_Registry.RemoveComponent<t_ComponentType>(m_RegistryEntityID);
		}

		void RemoveComponent(ECSInternal::ComponentIdentifier identifier)
		{
			m_Registry->m_Registry.RemoveComponent(m_RegistryEntityID, identifier);
		}

		void RemoveCustomComponentData(Assets::AssetHandle projectComponentHandle);

		template<typename t_ComponentType>
		t_ComponentType& GetComponent()
		{
			KG_ASSERT(HasComponent<t_ComponentType>(), "Entity does not have the component!");

			ExpectedRef<t_ComponentType> componentRef = 
				m_Registry->m_Registry.GetComponent<t_ComponentType>(m_RegistryEntityID);
			KG_ASSERT(componentRef.has_value());

			return componentRef.value().get();
		}

		void* GetCustomComponentData(Assets::AssetHandle projectComponentHandle);

	public:
		//==============================
		// Query State
		//==============================
		template<typename t_ComponentType>
		bool HasComponent()
		{
			return m_Registry->m_Registry.HasComponent<t_ComponentType>(m_RegistryEntityID);
		}

		bool HasComponent(ECSInternal::ComponentIdentifier compIdentifier)
		{
			m_Registry->m_Registry.HasComponent(m_RegistryEntityID, compIdentifier);
		}

		bool HasCustomComponentData(Assets::AssetHandle projectComponentHandle);

		UUID GetUUID() { return GetComponent<IDComponent>().m_ID; }
		const char* GetName() { return GetComponent<TagComponent>().m_Tag; }
		ECSInternal::EntityID GetInternalID() const { return m_RegistryEntityID;}

		bool IsValid() const
		{
			return m_RegistryEntityID != ECSInternal::k_InvalidEntityID && m_Registry != nullptr;
		}

	public:
		//==============================
		// Operator Overloads
		//==============================
		operator bool() const { return IsValid(); }
		operator uint32_t() const { return static_cast<uint32_t>(m_RegistryEntityID); }
		operator uint64_t() const { return static_cast<uint64_t>(m_RegistryEntityID); }
		bool operator==(const Entity& other) const
		{
			return m_RegistryEntityID == other.m_RegistryEntityID && m_Registry == other.m_Registry;
		}
		bool operator!=(const Entity& other) const { return !(*this == other); }

	private:
		//==============================
		// Internal Fields
		//==============================
		ECSInternal::EntityID m_RegistryEntityID { ECSInternal::k_InvalidEntityID };
		Registry* m_Registry { nullptr };
	};
}