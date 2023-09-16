#include "kgpch.h"

#include "Kargono/Scene/Entity.h"

namespace Kargono
{
	Entity::Entity(entt::entity handle, Scene* scene)
		: m_EntityHandle(handle), m_Scene(scene)
	{
		
	}
}
