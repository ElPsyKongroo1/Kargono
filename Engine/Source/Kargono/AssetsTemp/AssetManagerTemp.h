#pragma once
#include "Kargono/Core/Base.h"
#include "Kargono/Assets/Asset.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Utility/FileSystem.h"

#include "API/Serialization/yamlcppAPI.h"

#include <bitset>
#include <tuple>

// Functions to call

// Virtual functions to inject functionality


//==============================
// Manage Registry
//==============================

// DeserializeRegistry
// SerializeRegistry
// Clear Registry
// GetAssetRegistry()

//==============================
// Import Asset
//==============================

// ImportNewAssetFromFile
// ImportNewAssetFromData()

//==============================
// Read Asset from Registry
//==============================

// GetAsset(handle)

enum AssetManagerOptions : uint8_t
{
	None = 0, // Default value
	HasAssetCache = 1, // Store cache of the filetype in runtime memory for easy reuse
	HasIntermediateLocation = 2, // Specify that this asset manager generates an intermediate file to be stored in the project's Intermediates directory
	HasFileLocation = 3, // Specify that this asset manager stores a file somewhere in the project's Assets directory
	HasFileImporting = 4 // Specify that this asset manager is capable of importing the asset into the system from an external file
};

namespace Kargono::Assets
{
	using AssetRegistry = std::unordered_map<AssetHandle, Assets::Asset>;

	template <typename AssetType>
	class AssetManagerTemp
	{
	public:
		Ref<AssetType> GetAsset(const AssetHandle& handle)
		{
			KG_ASSERT(Projects::ProjectService::GetActive(), "There is no active project when retrieving asset!");

			if (m_Flags.test(AssetManagerOptions::HasAssetCache))
			{
				if (m_AssetCache.contains(handle))
				{
					return m_AssetCache[handle];
				}
			}

			if (m_AssetRegistry.contains(handle))
			{
				auto asset = m_AssetRegistry[handle];
				std::filesystem::path assetPath = Projects::ProjectService::GetActiveAssetDirectory() / 
					(m_Flags.test(AssetManagerOptions::HasIntermediateLocation) ? asset.Data.IntermediateLocation : asset.Data.FileLocation);
				Ref<AssetType> newAsset = InstantiateAssetIntoMemory(asset, assetPath);
				if (m_Flags.test(AssetManagerOptions::HasAssetCache))
				{
					m_AssetCache.insert({ asset.Handle, newAsset });
				}
				return newAsset;
			}

			KG_WARN("No {} asset is associated with {} handle. Returning an empty asset reference", m_AssetName, handle);
			return nullptr;
		}

		std::tuple<AssetHandle, Ref<AssetType>> GetAsset(const std::filesystem::path& fileLocation)
		{
			KG_ASSERT(Projects::ProjectService::GetActive(), "Attempt to use Project Field without active project!");
			KG_ASSERT(m_Flags.test(AssetManagerOptions::HasFileLocation), 
				"Attempt to retrieve a asset using a file location when this asset type does not support storing file locations");

			std::filesystem::path assetPath = fileLocation;

			if (fileLocation.is_absolute())
			{
				assetPath = Utility::FileSystem::GetRelativePath(Projects::ProjectService::GetActiveAssetDirectory(), fileLocation);
			}

			for (auto& [assetHandle, asset] : m_AssetRegistry)
			{
				if (asset.Data.FileLocation.compare(assetPath) == 0)
				{
					return { assetHandle, GetAsset(assetHandle) };
				}
			}
			// Return empty asset if the asset is not found in the registry
			KG_WARN("Invalid filepath provided to GetAsset(filepath) {}. Returning empty {} asset.", fileLocation.string(), m_AssetName);
			return {Assets::EmptyHandle, nullptr};
		}
		std::filesystem::path GetAssetFileLocation(const AssetHandle& handle)
		{
			KG_ASSERT(m_Flags.test(AssetManagerOptions::HasFileLocation),
				"Attempt to retrieve an asset's file location when none is support by the defined asset type.");
			if (!m_AssetRegistry.contains(handle))
			{
				KG_WARN("Could not locate {} asset when attempting to retrieve it's file location using {} handle", m_AssetName, handle);
				return std::filesystem::path();
			}
			return m_AssetRegistry[handle].Data.FileLocation;
		}

		std::filesystem::path GetAssetIntermediateLocation(const AssetHandle& handle)
		{
			KG_ASSERT(m_Flags.test(AssetManagerOptions::HasIntermediateLocation), 
				"Attempt to retrieve asset intermediate when none is generated for defined type");
			if (!m_AssetRegistry.contains(handle))
			{
				KG_WARN("Could not locate {} asset when attempting to retrieve it's intermediate location with {} handle", m_AssetName, handle);
				return std::filesystem::path();
			}
			return m_AssetRegistry[handle].Data.IntermediateLocation;
		}


		bool HasAsset(const AssetHandle& handle)
		{
			return m_AssetRegistry.contains(handle);
		}

		void ClearAssetRegistry()
		{
			if (m_Flags.test(AssetManagerOptions::HasAssetCache))
			{
				m_AssetCache.clear();
			}
			m_AssetRegistry.clear();
		}

		AssetHandle ImportAssetFromFile(const std::filesystem::path& filePath)
		{
			KG_ASSERT(m_Flags.test(AssetManagerOptions::HasFileImporting), "Attempt to import an asset for a file type that does not support importing");

			// Create Checksum
			const std::string currentCheckSum = Utility::FileSystem::ChecksumFromFile(filePath);

			// Ensure checksum is valid
			if (currentCheckSum.empty())
			{
				KG_WARN("Generated empty checksum from file at {}", filePath.string());
				return Assets::EmptyHandle;
			}

			// Ensure duplicate asset is not found in registry.
			for (const auto& [handle, asset] : m_AssetRegistry)
			{
				if (asset.Data.CheckSum == currentCheckSum)
				{
					KG_WARN("Attempt to instantiate duplicate {} asset. Returning existing asset.", m_AssetName);
					return handle;
				}
			}

			// Check if file extension is appropriate for this file type
			bool foundValidExtension{ false };
			for (auto& extension : m_ValidImportFileExtensions)
			{
				if (filePath.extension().string() == extension)
				{
					foundValidExtension = true;
					break;
				}
			}
			
			// Exit if extension is invalid
			if (!foundValidExtension)
			{
				KG_WARN("Invalid file extension ({}) for {} asset type. Valid file extensions for {} include: ");
				for (auto& extension : m_ValidImportFileExtensions)
				{
					KG_WARN("  {}", extension);
				}
				return Assets::EmptyHandle;
			}

			// Create New Asset/Handle
			AssetHandle newHandle { Assets::EmptyHandle };
			Assets::Asset newAsset{};
			newAsset.Handle = newHandle;

			// Check if intermediates are used. If so, generate the intermediate.
			if (m_Flags.test(AssetManagerOptions::HasIntermediateLocation))
			{
				//CreateAssetIntermediateFromFile(filePath, newAsset);
				newAsset.Data.CheckSum = currentCheckSum;
			}
			else
			{
				KG_ASSERT(true, "Attempt to import a file that does not generate an intermediate. I have not decided what happens in this case.");
			}
			// TODO: Make sure to modify the code below if fixing the asset above
			std::filesystem::path assetPath = Projects::ProjectService::GetActiveAssetDirectory() /
				(m_Flags.test(AssetManagerOptions::HasIntermediateLocation) ? newAsset.Data.IntermediateLocation : newAsset.Data.FileLocation);

			// Add new asset into asset registry
			m_AssetRegistry.insert({ newHandle, newAsset });
			//SerializeAssetRegistry();

			// Fill in-memory cache if appropriate
			if (m_Flags.test(AssetManagerOptions::HasAssetCache))
			{
				m_AssetCache.insert({ newHandle, InstantiateAssetIntoMemory(newAsset, assetPath) });
			}
			return newHandle;
		}

		virtual Ref<AssetType> InstantiateAssetIntoMemory(Assets::Asset& asset, const std::filesystem::path& assetPath) = 0;

	protected:
		std::string m_AssetName{ "Uninitialized Asset Name" };
		std::string m_FileExtension { ".kgfile" };
		std::vector<std::string> m_ValidImportFileExtensions{};
		std::unordered_map<AssetHandle, Assets::Asset> m_AssetRegistry{};
		std::unordered_map<AssetHandle, Ref<AssetType>> m_AssetCache{};
		std::bitset<8> m_Flags {0b00000000};
	};
}
