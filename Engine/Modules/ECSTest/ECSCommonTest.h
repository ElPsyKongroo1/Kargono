#pragma once

#include "cstdint"
#include "bitset"

namespace Kargono::ECS
{
	// Entity types
	using EntityID = uint32_t;
	using EntityCount = EntityID;
	constexpr EntityID k_MaxEntities{ 5'000 }; // Arbitrary

	// Component types
	using ComponentType = uint8_t;
	constexpr ComponentType k_MaxComponents{ 32 }; // Arbitrary
	using ComponentIndex = size_t;
	using ComponentCount = ComponentIndex;

	using Signature = std::bitset<k_MaxComponents>;
}

