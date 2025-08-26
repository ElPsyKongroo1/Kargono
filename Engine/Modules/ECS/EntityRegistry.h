#pragma once

#include "Kargono/Core/UUID.h"

#include "Modules/ECSTest/RegistryTest.h"

#include <unordered_map>


namespace Kargono::ECS
{
	struct EntityRegistry
	{
		Registry* m_Registry{ nullptr };
		std::unordered_map<UUID, ECS::EntityID> m_EntityMap;
	};
}
