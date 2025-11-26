#pragma once

#include "Modules/Modules/ModulesCommon.h"
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
	using AssetIdentifier = Modules::TypeIdentifier;
	constexpr AssetIdentifier k_InvalidAssetIdentifier
	{
		std::numeric_limits<AssetIdentifier>::max()
	};

	constexpr UUID k_EmptyHandle{ 0 };

	class AssetHandle
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		constexpr AssetHandle() : m_Handle(k_EmptyHandle) {}
		constexpr AssetHandle(uint64_t handle) : m_Handle(handle) {}
		constexpr AssetHandle(UUID handle) : m_Handle(handle) {}
		constexpr AssetHandle(const AssetHandle&) = default;
	public:
		//==============================
		// Handle State
		//==============================
		bool IsValid() const
		{
			return m_Handle != k_EmptyHandle;
		}
	public:
		//==============================
		// Operator Overloads
		//==============================
		operator uint64_t() const 
		{ 
			return m_Handle; 
		}
		operator UUID() const 
		{ 
			return m_Handle; 
		}
		operator std::string() const 
		{ 
			return std::to_string((uint64_t)m_Handle); 
		}
	public:
		//==============================
		// Public Fields
		//==============================
		UUID m_Handle{ k_EmptyHandle };
	};

	enum AssetFlag : uint8_t
	{
		None = 0, // Default value
		HasAssetCache = 1, // Store cache of the filetype in runtime memory for easy reuse
		RequireUniqueName = 2, // Individual asset names must be unique
		RequireUniqueHash = 3 // Individual asset hashes must be unique
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

namespace std
{
	template<>
	struct hash<Kargono::Assets::AssetHandle>
	{
		std::size_t operator()(const Kargono::Assets::AssetHandle& handle) const
		{
			return (uint64_t)handle;
		}
	};
}