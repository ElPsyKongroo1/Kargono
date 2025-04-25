#pragma once

#include "Kargono/Core/UUID.h"
#include "ECSModule/EngineComponents.h"
#include "ECSModule/EntityRegistry.h"
#include "ECSModule/ExternalAPI/enttAPI.h"

namespace Kargono::ECS
{
	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, EntityRegistry* registry);
		Entity(const Entity& other) = default;

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			KG_ASSERT(!HasComponent<T>(), "Entity already has component!");
			T& component = m_Registry->m_EnTTRegistry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
			return component;
		}

		void AddProjectComponentData(Assets::AssetHandle projectComponentHandle);

		template<typename T, typename... Args>
		T& AddOrReplaceComponent(Args&&... args)
		{
			T& component = m_Registry->m_EnTTRegistry.emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);
			return component;
		}

		template<typename T>
		T& GetComponent()
		{
			KG_ASSERT(HasComponent<T>(), "Entity does not have the component!")

			return m_Registry->m_EnTTRegistry.get<T>(m_EntityHandle);
		}

		void* GetProjectComponentData(Assets::AssetHandle projectComponentHandle);

		template<typename T>
		bool HasComponent()
		{
			return m_Registry->m_EnTTRegistry.all_of<T>(m_EntityHandle);
		}

		bool HasProjectComponentData(Assets::AssetHandle projectComponentHandle);

		template<typename T>
		void RemoveComponent()
		{
			KG_ASSERT(HasComponent<T>(), "Entity does not have the component!");
			m_Registry->m_EnTTRegistry.remove<T>(m_EntityHandle);
		}

		void RemoveProjectComponentData(Assets::AssetHandle projectComponentHandle);

		operator bool() const { return m_EntityHandle != entt::null; }
		operator entt::entity() const { return m_EntityHandle; }
		operator uint32_t() const { return static_cast<uint32_t>(m_EntityHandle); }
		operator uint64_t() const { return static_cast<uint64_t>(m_EntityHandle); }

		UUID GetUUID() { return GetComponent<IDComponent>().ID; }
		const std::string& GetName() { return GetComponent<TagComponent>().Tag; }

		bool operator==(const Entity& other) const
		{
			return m_EntityHandle == other.m_EntityHandle && m_Registry == other.m_Registry;
		}

		bool operator!=(const Entity& other) const
		{
			return  !(*this == other);
		}
	private:
		entt::entity m_EntityHandle {entt::null};
		EntityRegistry* m_Registry { nullptr };
	};
}

namespace Kargono::Utility
{
	inline static std::unordered_map<std::string, std::function<bool(ECS::Entity)>> s_EntityHasComponentFunc {};

	template<typename... Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
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

	template<typename ... Component>
	static void RegisterHasComponent()
	{
		([]()
			{
				std::string fullName = typeid(Component).name();
				size_t pos = fullName.find_last_of(':');
				std::string componentName = fullName.substr(pos + 1);
				s_EntityHasComponentFunc[componentName] = [](ECS::Entity entity) { return entity.HasComponent<Component>(); };
			}(), ...);
	}

	template<typename ... Component>
	static void RegisterHasComponent(ECS::ComponentGroup<Component ...>)
	{
		RegisterHasComponent<Component ...>();
	}
}

