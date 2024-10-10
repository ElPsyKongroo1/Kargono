#include "kgpch.h"

#include "Kargono/ECS/Entity.h"
#include "Kargono/ECS/ProjectComponent.h"
#include "Kargono/Assets/AssetService.h"

namespace Kargono::ECS
{
	Entity::Entity(entt::entity handle, EntityRegistry* registry)
	{
		if (!registry->m_EnTTRegistry.valid(handle))
		{
			KG_WARN("Invalid entity trying to be created with handle {0} and registry pointer {1}", (int32_t)handle, (void*)registry);
			return;
		}

		m_EntityHandle = handle;
		m_Registry = registry;
	}
	void Entity::AddProjectComponent(Assets::AssetHandle projectComponentHandle)
	{
		Ref<ProjectComponent> projectComponent = Assets::AssetService::GetProjectComponent(projectComponentHandle);
		KG_ASSERT(projectComponent);

		// Create new project component
		ProjectComponentStorage& storage = m_Registry->m_ProjectComponentStorage.at(projectComponent->m_BufferSlot);
		storage.m_AddProjectComponent(storage.m_EnTTStorageReference, m_EntityHandle);

		// Set initial values of data
		uint8_t* componentReference = (uint8_t*)storage.m_GetProjectComponent(storage.m_EnTTStorageReference, m_EntityHandle);
		for (size_t iteration{0}; iteration < projectComponent->m_DataLocations.size(); iteration++)
		{
			Utility::InitializeDataForWrappedVarBuffer(
				projectComponent->m_DataTypes.at(iteration),
				componentReference + projectComponent->m_DataLocations.at(iteration));
		}

	}
	void* Entity::GetProjectComponent(Assets::AssetHandle projectComponentHandle)
	{
		Ref<ProjectComponent> projectComponent = Assets::AssetService::GetProjectComponent(projectComponentHandle);
		KG_ASSERT(projectComponent);
		ProjectComponentStorage& storage = m_Registry->m_ProjectComponentStorage.at(projectComponent->m_BufferSlot);
		return storage.m_GetProjectComponent(storage.m_EnTTStorageReference, m_EntityHandle);
	}
	bool Entity::HasProjectComponent(Assets::AssetHandle projectComponentHandle)
	{
		Ref<ProjectComponent> projectComponent = Assets::AssetService::GetProjectComponent(projectComponentHandle);
		KG_ASSERT(projectComponent);
		ProjectComponentStorage& storage = m_Registry->m_ProjectComponentStorage.at(projectComponent->m_BufferSlot);
		return storage.m_CheckEntityExists(storage.m_EnTTStorageReference, m_EntityHandle);
	}
	void Entity::RemoveProjectComponent(Assets::AssetHandle projectComponentHandle)
	{
		Ref<ProjectComponent> projectComponent = Assets::AssetService::GetProjectComponent(projectComponentHandle);
		KG_ASSERT(projectComponent);
		ProjectComponentStorage& storage = m_Registry->m_ProjectComponentStorage.at(projectComponent->m_BufferSlot);
		storage.m_RemoveProjectComponent(storage.m_EnTTStorageReference, m_EntityHandle);
	}
}
