#pragma once

#include "cstdint"
#include "bitset"

namespace Kargono::ECS
{
	// Entity types
	using EntityID = size_t;
	using EntityCount = EntityID;
	constexpr EntityID k_MaxEntities{ 5'000 }; // Arbitrary

	// Component types
	using ComponentMask = uint8_t;
	constexpr ComponentMask k_MaxComponents{ 32 }; // Arbitrary
	using ComponentIndex = size_t;
	using ComponentCount = ComponentIndex;

	using Signature = std::bitset<(size_t)k_MaxComponents>;
}

