#include "kgpch.h"

#include "Kargono/Scenes/Entity.h"

namespace Kargono::Scenes
{
	Entity::Entity(entt::entity handle, Scene* scene)
	{
		if (!scene->IsEntityValid(handle))
		{
			KG_WARN("Invalid entity trying to be created with handle {0} and scene pointer {1}", (int32_t)handle, (void*)scene);
			return;
		}

		m_EntityHandle = handle;
		m_Scene = scene;

	}
}
