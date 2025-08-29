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
		// Public Fields
		//==============================
		ECSInternal::RegistryInternal m_Registry{};
		EntityUUIDMap m_EntityMap;
	};
}
