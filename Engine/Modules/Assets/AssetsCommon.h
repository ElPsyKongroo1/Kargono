#pragma once

#include "Modules/Core/Module.h"

#include "Kargono/Core/UUID.h"
#include "Kargono/Core/FixedBufferString.h"
#include "Kargono/Core/BitField.h"

#include <cstdint>
#include <limits>

namespace Kargono::Assets
{
	// Unique identifier for each type of asset
	using AssetIdentifier = ModuleTypeIdentifier;
	constexpr AssetIdentifier k_InvalidAssetIdentifier
	{
		std::numeric_limits<AssetIdentifier>::max()
	};

	// An AssetHandle is a unique identifier for a particular asset instance
	using AssetHandle = Kargono::UUID;
	constexpr uint64_t k_EmptyHandle{ 0 };

	enum AssetFlags : uint8_t
	{
		None = 0, // Default value
		HasAssetCache = 1, // Store cache of the filetype in runtime memory for easy reuse
		HasIntermediateLocation = 2, // Specify that this asset manager generates an intermediate file to be stored in the project's Intermediates directory
		HasFileLocation = 3, // Specify that this asset manager stores a file somewhere in the project's Assets directory
		HasFileImporting = 4, // Specify that this asset manager is capable of importing the asset into the system from an external file
		HasAssetSaving = 5, // Specify that this asset manager is capable of saving to the underlying file data for it's type of asset
		HasAssetCreationFromName = 6 // Specify that this asset manager is capable of creating the underlying file data for it's type of asset
	};

	using ImportExtensionList = std::array<FixedBufStr16, 10>;

	struct AssetConfig
	{
	public:
		AssetIdentifier m_Identifier{ k_InvalidAssetIdentifier };
		FixedBufStr16 m_Name{ "Default Asset" };
		FixedBufStr16 m_FileExtension{ ".kgfile" };
		FixedBufStr16 m_IntermediateExtension{ ".kgbinary" };
		FixedBufStr256 m_RegistryPath{ "" };
		ImportExtensionList m_ImportExtensions{};
		BitField<uint8_t> m_Flags{ 0b00000000 };
	};

	enum class LoadState : uint8_t
	{
		Unloaded = 0,
		Loading,
		Loaded
	};
}