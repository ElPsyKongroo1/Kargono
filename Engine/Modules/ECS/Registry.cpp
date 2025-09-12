#include "kgpch.h"

#include "Modules/ECS/Registry.h"
#include "Modules/ECSInternal/CustomComponent.h"

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

	bool Registry::RegisterCustomComponent(Assets::AssetHandle customComponentHandle)
	{
		Ref<ECSInternal::CustomComponent> component = Assets::AssetService::GetCustomComponent(customComponentHandle);
		KG_ASSERT(component);

		if (component->m_ComponentSize == 0 || m_Registry.IsComponentRegistered(component->m_Identifier))
		{
			return;
		}

		// Register component
		ECSInternal::ComponentMetadata metadata = component->GenerateMetadata(customComponentHandle);
		m_Registry.RegisterComponent(component->m_Identifier, metadata);
	}
	bool Registry::UnRegisterCustomComponent(Assets::AssetHandle customComponentHandle)
	{
		Ref<ECSInternal::CustomComponent> component = Assets::AssetService::GetCustomComponent(customComponentHandle);
		KG_ASSERT(component);

		if (component->m_ComponentSize == 0 || !m_Registry.IsComponentRegistered(component->m_Identifier))
		{
			return;
		}

		// Clear the component store
		m_Registry.ClearComponentStore(component->m_Identifier);
	}
	size_t Registry::GetCustomComponentCount(Assets::AssetHandle customComponentHandle)
	{
		Ref<ECSInternal::CustomComponent> component = Assets::AssetService::GetCustomComponent(customComponentHandle);
		KG_ASSERT(component);

		if (component->m_ComponentSize == 0 || !m_Registry.IsComponentRegistered(customComponentHandle));
		{
			return 0;
		}

		KG_ASSERT(component->m_Identifier != ECSInternal::k_InvalidComponentIdentifier);

		return m_Registry.GetComponentCount(component->m_Identifier);
	}
}