#pragma once

#include "Kargono/Core/UUID.h"

#include "API/EntityComponentSystem/enttAPI.h"

#include <unordered_map>


namespace Kargono::ECS
{

	struct ProjectComponentStorage
	{
		void* m_EnTTStorageReference{ nullptr };
		// Bunch of function pointers for various functionality
		bool (*m_CheckEntityExists)(void*, entt::entity) = nullptr;
		void* (*m_GetProjectComponent)(void*, entt::entity) = nullptr;
		void (*m_AddProjectComponent)(void*, entt::entity) = nullptr;
		void (*m_RemoveProjectComponent)(void*, entt::entity) = nullptr;
	};

	struct EntityRegistry
	{
		entt::registry m_EnTTRegistry;
		std::unordered_map<UUID, entt::entity> m_EntityMap;
		std::vector<ProjectComponentStorage> m_ProjectComponentStorage;
	};

	class EntityRegistryService
	{
	public:
		static void RegisterProjectComponentWithEnTTRegistry(ECS::ProjectComponentStorage& newStorage, EntityRegistry& entityRegistry, size_t bufferSize, const std::string& componentName);
	};
}
