#pragma once

#include "Kargono/Core/UUID.h"

#include "API/EntityComponentSystem/enttAPI.h"

#include <unordered_map>


namespace Kargono::ECS
{
	struct EntityRegistry
	{
		entt::registry m_EnTTRegistry;
		std::unordered_map<UUID, entt::entity> m_EntityMap;
	};
}
