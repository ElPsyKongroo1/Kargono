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
	};

	struct EntityRegistry
	{
		entt::registry m_EnTTRegistry;
		std::unordered_map<UUID, entt::entity> m_EntityMap;
		std::vector<ProjectComponentStorage> m_ProjectStorage;
	};

	class EntityRegistryService
	{
	public:
		static void* GenerateEnTTStorageReference(EntityRegistry& entityRegistry, size_t bufferSize, const std::string& componentName);
	};
}
