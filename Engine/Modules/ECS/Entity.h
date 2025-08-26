#pragma once

#include "Kargono/Core/UUID.h"
#include "Modules/ECS/EngineComponents.h"
#include "Modules/ECS/EntityRegistry.h"
#include "Modules/ECS/ExternalAPI/enttAPI.h"
#include "Modules/ECSTest/RegistryTest.h"

namespace Kargono::ECS
{
	class Entity
	{
	public:
		Entity() = default;
		Entity(ECS::EntityID handle, EntityRegistry* registry);
		Entity(const Entity& other) = default;

	public:
		template<typename t_ComponentType, typename... t_Args>
		t_ComponentType& AddComponent(t_Args&&... args)
		{
			KG_ASSERT(!HasComponent<t_ComponentType>(), "Entity already has component!");
			t_ComponentType& component = 
				m_Registry->m_Registry->EmplaceComponent<t_ComponentType>
				(m_RegistryEntityID, std::forward<t_Args>(args)...);
			return component;
		}

		void AddProjectComponentData(Assets::AssetHandle projectComponentHandle);

		template<typename t_ComponentType, typename... t_Args>
		t_ComponentType& AddOrReplaceComponent(t_Args&&... args)
		{
			t_ComponentType& component
			{ 
				m_Registry->m_Registry->EmplaceOrReplaceComponent<t_ComponentType>(
					m_RegistryEntityID, std::forward<t_Args>(args)...) 
			};
			return component;
		}

		template<typename t_ComponentType>
		t_ComponentType& GetComponent()
		{
			KG_ASSERT(HasComponent<t_ComponentType>(), "Entity does not have the component!");

			ExpectedRef<t_ComponentType> componentRef = 
				m_Registry->m_Registry->GetComponent<t_ComponentType>(m_RegistryEntityID);
			KG_ASSERT(componentRef.has_value());

			return componentRef.value().get();
		}

		void* GetProjectComponentData(Assets::AssetHandle projectComponentHandle);

		template<typename t_ComponentType>
		bool HasComponent()
		{
			m_Registry->m_Registry->HasComponent<t_ComponentType>(m_RegistryEntityID);
		}

		bool HasComponent(EntityID entityID, ComponentIdentifier compIdentifier)
		{
			m_Registry->m_Registry->HasComponent(entityID, compIdentifier);
		}

		bool HasProjectComponentData(Assets::AssetHandle projectComponentHandle);

		template<typename t_ComponentType>
		void RemoveComponent()
		{
			KG_ASSERT(HasComponent<t_ComponentType>(), "Entity does not have the component!");
			m_Registry->m_Registry.RemoveComponent<t_ComponentType>(m_RegistryEntityID);
		}

		void RemoveComponent(ComponentIdentifier identifier)
		{
			m_Registry->m_Registry->RemoveComponent(m_RegistryEntityID, identifier);
		}


		void RemoveProjectComponentData(Assets::AssetHandle projectComponentHandle);

		operator bool() const { return m_RegistryEntityID != k_InvalidEntityID; }
		operator ECS::EntityID() const { return m_RegistryEntityID; }
		operator uint32_t() const { return static_cast<uint32_t>(m_RegistryEntityID); }
		operator uint64_t() const { return static_cast<uint64_t>(m_RegistryEntityID); }

		UUID GetUUID() { return GetComponent<IDComponent>().ID; }
		const std::string& GetName() { return GetComponent<TagComponent>().Tag; }

		bool operator==(const Entity& other) const
		{
			return m_RegistryEntityID == other.m_RegistryEntityID && m_Registry == other.m_Registry;
		}

		bool operator!=(const Entity& other) const
		{
			return  !(*this == other);
		}
	private:
		ECS::EntityID m_RegistryEntityID { ECS::k_InvalidEntityID };
		EntityRegistry* m_Registry { nullptr };
	};
}

namespace Kargono::Utility
{
	template<typename... Component>
	static void CopyAllComponents(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		([&]()
			{
				auto view = src.view<Component>();
				for (auto srcEntity : view)
				{
					entt::entity dstEntity = enttMap.at(src.get<ECS::IDComponent>(srcEntity).ID);

					auto& srcComponent = src.get<Component>(srcEntity);
					dst.emplace_or_replace<Component>(dstEntity, srcComponent);
				}
			}(), ...);
	}

	template<typename... Component>
	static void CopyComponent(ECS::ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		CopyComponent<Component...>(dst, src, enttMap);
	}

	template<typename... Component>
	static void CopyComponentIfExists(ECS::Entity dst, ECS::Entity src)
	{
		([&]()
			{
				if (src.HasComponent<Component>())
					dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
			}(), ...);
	}

	template<typename... Component>
	static void CopyComponentIfExists(ECS::ComponentGroup<Component...>, ECS::Entity dst, ECS::Entity src)
	{
		CopyComponentIfExists<Component...>(dst, src);
	}
}

