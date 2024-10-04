#include "kgpch.h"

#include "Kargono/ECS/Entity.h"

namespace Kargono::ECS
{
	Entity::Entity(entt::entity handle, entt::registry* registry)
	{
		if (!registry->valid(handle))
		{
			KG_WARN("Invalid entity trying to be created with handle {0} and registry pointer {1}", (int32_t)handle, (void*)registry);
			return;
		}

		m_EntityHandle = handle;
		m_Registry = registry;
	}
}
