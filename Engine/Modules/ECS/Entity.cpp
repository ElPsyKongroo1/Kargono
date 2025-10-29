#include "kgpch.h"

#include "Modules/ECS/Entity.h"
#include "Modules/ECSInternal/Assets/CustomComponent.h"
#include "Modules/Assets/AssetService.h"

namespace Kargono::ECS
{
	void Entity::Serialize(void* context)
	{

	}

	void Entity::Deserialize(void* context)
	{

	}

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
	void Entity::AddCustomComponentData(Assets::AssetHandle componentHandle)
	{
		// Get the custom component
		Ref<ECSInternal::CustomComponent> component = Assets::AssetService::GetCustomComponent(componentHandle);
		KG_ASSERT(component);
		KG_ASSERT(component->m_Identifier != ECSInternal::k_InvalidComponentIdentifier);
		if (component->m_ComponentSize == 0)
		{
			return;
		}


		uint8_t* componentReference
		{
			(uint8_t*)m_Registry->m_Registry.CreateComponent(m_RegistryEntityID, component->m_Identifier)
		};
		KG_ASSERT(componentReference);

		// Set initial values of data
		for (size_t iteration{0}; iteration < component->m_DataOffsets.size(); iteration++)
		{
			Utility::InitializeDataForWrappedVarBuffer(
				component->m_DataTypes.at(iteration),
				componentReference + component->m_DataOffsets.at(iteration));
		}

	}
	void* Entity::GetCustomComponentData(Assets::AssetHandle componentHandle)
	{
		Ref<ECSInternal::CustomComponent> component = Assets::AssetService::GetCustomComponent(componentHandle);
		KG_ASSERT(component);
		KG_ASSERT(component->m_Identifier != ECSInternal::k_InvalidComponentIdentifier);
		if (component->m_ComponentSize == 0)
		{
			return nullptr;
		} 

		return m_Registry->m_Registry.GetComponent(m_RegistryEntityID, component->m_Identifier);
	}
	bool Entity::HasCustomComponentData(Assets::AssetHandle componentHandle)
	{
		Ref<ECSInternal::CustomComponent> component = Assets::AssetService::GetCustomComponent(componentHandle);
		KG_ASSERT(component);
		if (component->m_ComponentSize == 0)
		{
			return false;
		}
		return m_Registry->m_Registry.HasComponent(m_RegistryEntityID, component->m_Identifier);
	}
	void Entity::RemoveCustomComponentData(Assets::AssetHandle componentHandle)
	{
		Ref<ECSInternal::CustomComponent> component = Assets::AssetService::GetCustomComponent(componentHandle);
		KG_ASSERT(component);
		KG_ASSERT(component->m_Identifier != ECSInternal::k_InvalidComponentIdentifier);
		if (component->m_ComponentSize == 0)
		{
			return;
		}

		m_Registry->m_Registry.RemoveComponent(m_RegistryEntityID, component->m_Identifier);
	}
}
