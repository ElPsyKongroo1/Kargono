#pragma once

#include "Modules/ECSInternal/ECSInternalCommon.h"
#include "Modules/ECSInternal/RegistryInternal.h"
#include "Modules/ECSInternal/Module/ECSInternalModule.h"
#include "Modules/ECSInternal/EntityRegistry.h"
#include "Modules/Memory/IAllocator.h"

#include <array>
#include <unordered_map>
#include <cstdint>

namespace Kargono::ECSInternal
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
		virtual void Init(EntityRegistry* entityRegistry, Memory::IAllocator* allocator,
			ComponentMetadata metadata) = 0;
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
		virtual ComponentCount GetComponentCount() = 0;

		//==============================
		// Getters/Setters
		//==============================
		virtual void SetComponentMetadata(const ComponentMetadata& metadata) = 0;
		virtual const ComponentMetadata& GetComponentMetadata() const = 0;
	};
}