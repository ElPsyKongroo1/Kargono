#pragma once

#include "Kargono/Core/BitField.h"

#include <cstdint>
#include <bitset>
#include <limits>

namespace Kargono::ECS
{
	// Entity types
	using EntityID = uint32_t;
	using EntityCount = EntityID;
	constexpr EntityID k_MaxEntities{ 5'000 }; // Arbitrary
	constexpr EntityID k_InvalidEntityID{ std::numeric_limits<EntityID>::max() };

	// Component types
	using ComponentMask = uint32_t;
	constexpr ComponentMask k_MaxComponents{ 32 }; // Arbitrary
	using ComponentIndex = size_t;
	using ComponentCount = ComponentIndex;

	using Signature = BitField<uint32_t>;

	// This is to ensure that a signature can be succuessfully union'd w/
	// an entity ID in EntityRegistry.h
	static_assert(sizeof(Signature) == sizeof(EntityID));
}

