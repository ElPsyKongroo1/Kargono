#pragma once

#include "Modules/Core/Module.h"

#include "Kargono/Core/UUID.h"
#include "Kargono/Core/FixedBufferString.h"
#include "Kargono/Core/BitField.h"

#include <cstdint>
#include <type_traits>
#include <limits>
#include <string_view>
#include <filesystem>

namespace Kargono::Assets
{
	// Unique identifier for each type of asset
	using AssetIdentifier = ModuleTypeIdentifier;
	constexpr AssetIdentifier k_InvalidAssetIdentifier
	{
		std::numeric_limits<AssetIdentifier>::max()
	};

	// An AssetHandle is a unique identifier for a particular asset instance
	using AssetHandle = UUID;
	constexpr uint64_t k_EmptyHandle{ 0 };

	enum AssetFlag : uint8_t
	{
		None = 0, // Default value
		HasAssetCache = 1, // Store cache of the filetype in runtime memory for easy reuse
		RequireUniqueName = 2, // Individual asset names must be unique
		AllowDefaultUpdateAsset = 3 // Allow updating an asset using an asset reference by default
	};

	using AssetFlags = BitField<std::underlying_type_t<AssetFlag>>;

	enum class LoadState : uint8_t
	{
		Unloaded = 0,
		Loading = 1,
		Loaded = 2
	};

	struct AssetCreationData
	{
		std::string_view m_AssetName{};
		std::filesystem::path m_CreationDirectory{};
		bool m_IsHidden{ false };
	};
}