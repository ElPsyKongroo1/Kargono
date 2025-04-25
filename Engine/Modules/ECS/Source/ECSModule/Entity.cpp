#include "kgpch.h"

#include "ECSModule/Entity.h"
#include "ECSModule/ProjectComponent.h"
#include "AssetModule/AssetService.h"

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
	void Entity::AddProjectComponentData(Assets::AssetHandle projectComponentHandle)
	{
		Ref<ProjectComponent> projectComponent = Assets::AssetService::GetProjectComponent(projectComponentHandle);
		KG_ASSERT(projectComponent);
		if (projectComponent->m_BufferSize == 0)
		{
			return;
		}
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
	void* Entity::GetProjectComponentData(Assets::AssetHandle projectComponentHandle)
	{
		Ref<ProjectComponent> projectComponent = Assets::AssetService::GetProjectComponent(projectComponentHandle);
		KG_ASSERT(projectComponent);
		if (projectComponent->m_BufferSize == 0)
		{
			return nullptr;
		}
		ProjectComponentStorage& storage = m_Registry->m_ProjectComponentStorage.at(projectComponent->m_BufferSlot);
		return storage.m_GetProjectComponent(storage.m_EnTTStorageReference, m_EntityHandle);
	}
	bool Entity::HasProjectComponentData(Assets::AssetHandle projectComponentHandle)
	{
		Ref<ProjectComponent> projectComponent = Assets::AssetService::GetProjectComponent(projectComponentHandle);
		KG_ASSERT(projectComponent);
		if (projectComponent->m_BufferSize == 0)
		{
			return false;
		}
		ProjectComponentStorage& storage = m_Registry->m_ProjectComponentStorage.at(projectComponent->m_BufferSlot);
		return storage.m_CheckEntityExists(storage.m_EnTTStorageReference, m_EntityHandle);
	}
	void Entity::RemoveProjectComponentData(Assets::AssetHandle projectComponentHandle)
	{
		Ref<ProjectComponent> projectComponent = Assets::AssetService::GetProjectComponent(projectComponentHandle);
		KG_ASSERT(projectComponent);
		if (projectComponent->m_BufferSize == 0)
		{
			return;
		}
		ProjectComponentStorage& storage = m_Registry->m_ProjectComponentStorage.at(projectComponent->m_BufferSlot);
		storage.m_RemoveProjectComponent(storage.m_EnTTStorageReference, m_EntityHandle);
	}
}
