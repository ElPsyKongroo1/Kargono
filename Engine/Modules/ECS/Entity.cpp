#include "kgpch.h"

#include "Modules/ECS/Entity.h"
#include "Modules/ECS/Components/ProjectComponent.h"
#include "Modules/Assets/AssetService.h"

namespace Kargono::ECS
{
	Entity::Entity(ECSInternal::EntityID handle, Registry* registry)
	{
		KG_ASSERT(registry);

		if (!registry->m_Registry.HasEntity(handle))
		{
			KG_WARN("Invalid entity trying to be created with handle {0} and registry pointer {1}", (int32_t)handle, (void*)registry);
			return;
		}

		m_RegistryEntityID = handle;
		m_Registry = registry;
	}
	void Entity::AddProjectComponentData(Assets::AssetHandle projectComponentHandle)
	{
		// Get the project component
		Ref<ProjectComponent> projectComponent = Assets::AssetService::GetProjectComponent(projectComponentHandle);
		KG_ASSERT(projectComponent);
		KG_ASSERT(projectComponent->m_Identifier != ECSInternal::k_InvalidComponentIdentifier);
		if (projectComponent->m_ComponentSize == 0)
		{
			return;
		}


		uint8_t* componentReference
		{
			(uint8_t*)m_Registry->m_Registry.CreateComponent(m_RegistryEntityID, projectComponent->m_Identifier)
		};
		KG_ASSERT(componentReference);

		// Set initial values of data
		for (size_t iteration{0}; iteration < projectComponent->m_DataOffsets.size(); iteration++)
		{
			Utility::InitializeDataForWrappedVarBuffer(
				projectComponent->m_DataTypes.at(iteration),
				componentReference + projectComponent->m_DataOffsets.at(iteration));
		}

	}
	void* Entity::GetProjectComponentData(Assets::AssetHandle projectComponentHandle)
	{
		Ref<ProjectComponent> projectComponent = Assets::AssetService::GetProjectComponent(projectComponentHandle);
		KG_ASSERT(projectComponent);
		KG_ASSERT(projectComponent->m_Identifier != ECSInternal::k_InvalidComponentIdentifier);
		if (projectComponent->m_ComponentSize == 0)
		{
			return nullptr;
		} 

		return m_Registry->m_Registry.GetComponent(m_RegistryEntityID, projectComponent->m_Identifier);
	}
	bool Entity::HasProjectComponentData(Assets::AssetHandle projectComponentHandle)
	{
		Ref<ProjectComponent> projectComponent = Assets::AssetService::GetProjectComponent(projectComponentHandle);
		KG_ASSERT(projectComponent);
		if (projectComponent->m_ComponentSize == 0)
		{
			return false;
		}
		return m_Registry->m_Registry.HasEntity(m_RegistryEntityID);
	}
	void Entity::RemoveProjectComponentData(Assets::AssetHandle projectComponentHandle)
	{
		Ref<ProjectComponent> projectComponent = Assets::AssetService::GetProjectComponent(projectComponentHandle);
		KG_ASSERT(projectComponent);
		KG_ASSERT(projectComponent->m_Identifier != ECSInternal::k_InvalidComponentIdentifier);
		if (projectComponent->m_ComponentSize == 0)
		{
			return;
		}

		m_Registry->m_Registry.RemoveComponent(m_RegistryEntityID, projectComponent->m_Identifier);
	}
}
