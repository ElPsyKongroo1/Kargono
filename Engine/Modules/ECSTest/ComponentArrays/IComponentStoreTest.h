#pragma once

#include "Modules/ECSTest/CommonTest.h"
#include "Modules/ECSTest/EntityRegistryTest.h"
#include "Modules/ECSTest/Module/ECSModule.h"

#include "Kargono/Memory/IAllocator.h"

#include <array>
#include <unordered_map>
#include <cstdint>

namespace Kargono::ECS
{
	class IComponentStore
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		IComponentStore() = default;
		virtual ~IComponentStore() = default;
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		virtual void Init(EntityRegistryTest* entityRegistry, Memory::IAllocator* allocator,
			size_t componentSize, size_t componentAlignment) = 0;
		virtual void Terminate() = 0;
		virtual void Clear() = 0;
	public:
		//==============================
		// Manage Components
		//==============================
		[[nodiscard]] virtual void* CreateComponent(EntityID entityID) = 0;
		[[nodiscard]] virtual bool RemoveComponent(EntityID entityID) = 0;
		[[nodiscard]] virtual void* GetComponent(EntityID entityID) = 0;

		//==============================
		// Query State
		//==============================
		virtual bool HasComponent(EntityID entityID) = 0;

		//==============================
		// Getters/Setters
		//==============================
		virtual void SetComponentFunctors(const ComponentFunctors& functors) = 0;
		virtual const ComponentFunctors& GetComponentFunctors() const = 0;
	};
}