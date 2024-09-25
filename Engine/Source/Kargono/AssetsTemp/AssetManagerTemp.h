#pragma once
#include "Kargono/Core/Base.h"
#include "Kargono/Assets/Asset.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Utility/FileSystem.h"

#include "API/Serialization/yamlcppAPI.h"

#include <bitset>

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
	None = 0,
	UseAssetCache = 1,
	CreateAssetIntermediate = 2
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

			if (m_Flags.test(AssetManagerOptions::UseAssetCache))
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
					(m_Flags.test(AssetManagerOptions::CreateAssetIntermediate) ? asset.Data.IntermediateLocation : asset.Data.FileLocation);
				Ref<AssetType> newAsset = InstantiateAssetIntoMemory(asset, assetPath);
				if (m_Flags.test(AssetManagerOptions::UseAssetCache))
				{
					m_AssetCache.insert({ asset.Handle, newAsset });
				}
				return newAsset;
			}

			KG_ERROR("No asset is associated with provided handle!");
			return nullptr;
		}

		std::filesystem::path GetAssetIntermediateLocation(const AssetHandle& handle)
		{
			if (!m_AssetRegistry.contains(handle))
			{
				KG_ERROR("Could not locate asset when attempting to retrieve it's intermediate location");
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
			if (m_Flags.test(AssetManagerOptions::UseAssetCache))
			{
				m_AssetCache.clear();
			}
			m_AssetRegistry.clear();
		}

		virtual Ref<AssetType> InstantiateAssetIntoMemory(Assets::Asset& asset, const std::filesystem::path& assetPath) = 0;

	protected:
		std::unordered_map<AssetHandle, Assets::Asset> m_AssetRegistry{};
		std::unordered_map<AssetHandle, Ref<AssetType>> m_AssetCache{};
		std::bitset<8> m_Flags {0b00000000};
	};
}
