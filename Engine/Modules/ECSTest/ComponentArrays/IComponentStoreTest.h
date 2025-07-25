#pragma once

#include "Modules/ECSTest/CommonTest.h"
#include "Kargono/Memory/IAllocator.h"

#include "Modules/ECSTest/EntityRegistryTest.h"

#include <array>
#include <unordered_map>
#include <cstdint>

namespace Kargono::ECS
{
	class IComponentStore
	{
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		virtual void Init(EntityRegistryTest* entityRegistry) = 0;
	public:
		//==============================
		// Manage Components
		//==============================
		[[nodiscard]] virtual bool InsertComponent(EntityID entityID, void* component) = 0;
		[[nodiscard]] virtual bool RemoveComponent(EntityID entityID) = 0;
		[[nodiscard]] virtual void* GetComponent(EntityID entityID) = 0;
	};
}