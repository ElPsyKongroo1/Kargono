#pragma once

#include "Kargono/Core/UUID.h"

#include "Modules/ECSInternal/RegistryInternal.h"

#include <unordered_map>


namespace Kargono::ECS
{
	using EntityUUIDMap = std::unordered_map<UUID, ECSInternal::EntityID>;

	struct Registry
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		Registry() = default;
		~Registry() = default;
	public:
		//==============================
		// Lifecycle Functions
		//==============================
		[[nodiscard]] bool Init(Memory::IAllocator* backingAlloc);
		[[nodiscard]] bool Terminate();

		//==============================
		// Component Registration
		//==============================
		// Handle custom component(s)
		[[nodiscard]] bool RegisterCustomComponent(Assets::AssetHandle customComponentHandle);
		[[nodiscard]] bool UnRegisterCustomComponent(Assets::AssetHandle customComponentHandle);
		// Handle templated components
		
		// Handle identifier components


		//==============================
		// Query State
		//==============================
		size_t GetCustomComponentCount(Assets::AssetHandle customComponentHandle);
	public:
		//==============================
		// Public Fields
		//==============================
		ECSInternal::RegistryInternal m_Registry{};
		EntityUUIDMap m_EntityMap;
	};
}
