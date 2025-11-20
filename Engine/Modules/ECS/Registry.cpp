#include "kgpch.h"

#include "Modules/ECS/Entity.h"
#include "Modules/ECS/Registry.h"
#include "Modules/ECSInternal/Assets/CustomComponent.h"

#include "Modules/Core/Components/Transform.h"
#include "Modules/Core/Components/Tag.h"

#include "Modules/Events/SceneEvent.h"

namespace Kargono::ECS
{
	bool Registry::Init(Memory::IAllocator* backingAlloc)
	{
		return m_Registry.Init(backingAlloc);
	}

	bool Registry::Terminate()
	{
		m_EntityMap.clear();
		return m_Registry.Terminate();
	}

	Entity Registry::CreateEntity(std::string_view name)
	{
		return CreateEntityWithUUID(RandomUUIDService::GetRandomUUID(), name);
	}
	Entity Registry::CreateEntityWithUUID(UUID uuid, std::string_view name = {})
	{
		ECS::Entity entity = { m_Registry.CreateEntity().value() , this };
		entity.SetUUID(uuid);
		entity.AddComponent<Transform>();
		Tag& tag = entity.AddComponent<Tag>();
		tag.m_Tag = name.empty() ? "Entity" : name;

		m_EntityMap[uuid] = entity.GetInternalID();

		Events::ManageEntity event = { entity.GetUUID(), this, Events::ManageEntityAction::Create };
		EngineService::GetActiveEngine().GetThread().OnEvent(&event);

		return entity;
	}

	Entity Registry::DuplicateEntity(Entity entity)
	{
		// Copy name because we're going to modify component data structure
		KG_ASSERT(entity.HasComponent<Tag>())
		std::string_view name = entity.GetComponent<Tag>().m_Tag.StringView();
		ECS::Entity newEntity = CreateEntity(name);

		// Copy over components
		m_Registry.CopyComponents(entity.GetInternalID(), newEntity.GetInternalID());

		return newEntity;
	}

	void Registry::DestroyEntity(ECS::Entity entity)
	{
		Events::ManageEntity event = { entity.GetUUID(), this , Events::ManageEntityAction::Delete };
		EngineService::GetActiveEngine().GetThread().OnEvent(&event);

		m_EntityMap.erase(entity.GetUUID());
		if (m_Registry.HasEntity(entity.GetInternalID()))
		{
			m_Registry.DestroyEntity(entity.GetInternalID());
		}
	}

	void Registry::ClearEntities()
	{
		m_Registry.Clear();
		m_EntityMap.clear();
	}

	Entity Registry::GetEntityByName(std::string_view name)
	{
		auto view = m_Registry.GetFlatView<Tag>();
		for (ECSInternal::EntityID entity : view)
		{
			const Tag& tc = m_Registry.GetComponent<Tag>(entity).value();
			if (tc.m_Tag.StringView() == name)
			{
				return Entity { entity, this };
			}
		}
		return {};
	}


	Entity Registry::GetEntityByUUID(UUID uuid)
	{
		if (!m_EntityMap.contains(uuid))
		{
			KG_WARN("Could not find entity by uuid");
			return {};
		}

		return { m_EntityMap.at(uuid), this };
	}

	Entity Registry::GetEntityByECSID(ECSInternal::EntityID id)
	{
		// Ensure enttID is valid for this scene's registry
		if (m_Registry.HasEntity(id))
		{
			return { id, this };
		}

		// Return empty entity
		return {};
	}

	bool Registry::IsEntityValid(ECSInternal::EntityID entity)
	{
		return m_Registry.HasEntity(entity);
	}

	UUID Registry::GetUUIDByName(std::string_view name)
	{
		auto view = m_Registry.GetFlatView<Tag>();
		for (ECSInternal::EntityID id : view)
		{
			Entity entity{ id, this };

			Tag& tagComponent{ entity.GetComponent<Tag>() };
			if (tagComponent.m_Tag.StringView() == name)
			{
				return entity.GetUUID();
			}
			
		}
		KG_WARN("Could not locate entity by name!");
		return Assets::k_EmptyHandle;
	}

	bool Registry::RegisterCustomComponent(Assets::AssetHandle customComponentHandle)
	{
		Assets::AssetRef<ECSInternal::CustomComponent> component = Assets::AssetService::m_CustomComponentManager.GetAssetByHandle(customComponentHandle);
		KG_ASSERT(component);

		if (component->m_ComponentSize == 0 || m_Registry.IsComponentRegistered(component->m_Identifier))
		{
			return false;
		}

		// Register component
		ECSInternal::ComponentMetadata metadata = component->GenerateMetadata(customComponentHandle);
		return m_Registry.RegisterComponent(component->m_Identifier, metadata);
	}
	bool Registry::UnRegisterCustomComponent(Assets::AssetHandle customComponentHandle)
	{
	    Assets::AssetRef<ECSInternal::CustomComponent> component = Assets::AssetService::m_CustomComponentManager.GetAssetByHandle(customComponentHandle);
		KG_ASSERT(component);

		if (component->m_ComponentSize == 0 || !m_Registry.IsComponentRegistered(component->m_Identifier))
		{
			return false;
		}

		// Clear the component store
		return m_Registry.ClearComponentStore(component->m_Identifier);
	}
	size_t Registry::GetCustomComponentCount(Assets::AssetHandle customComponentHandle)
	{
		Assets::AssetRef<ECSInternal::CustomComponent> component = Assets::AssetService::m_CustomComponentManager.GetAssetByHandle(customComponentHandle);
		KG_ASSERT(component);

		if (component->m_ComponentSize == 0 || !m_Registry.IsComponentRegistered(customComponentHandle));
		{
			return 0;
		}

		KG_ASSERT(component->m_Identifier != ECSInternal::k_InvalidComponentIdentifier);

		return m_Registry.GetComponentCount(component->m_Identifier);
	}
}