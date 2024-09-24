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
	UseRuntimeCache = 1
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
			KG_ASSERT(Projects::ProjectService::GetActive(), "There is no active project when retreiving asset!");

			if (m_Flags.test(AssetManagerOptions::UseRuntimeCache))
			{
				if (m_RuntimeCache.contains(handle))
				{
					return m_RuntimeCache[handle];
				}
			}

			if (m_AssetRegistry.contains(handle))
			{
				auto asset = m_AssetRegistry[handle];

				Ref<AssetType> newAsset = InstantiateAssetIntoMemory(asset);
				if (m_Flags.test(AssetManagerOptions::UseRuntimeCache))
				{
					m_RuntimeCache.insert({ asset.Handle, newAsset });
				}
				return newAsset;
			}

			KG_ERROR("No asset is associated with provided handle!");
			return nullptr;
		}

		virtual Ref<AssetType> InstantiateAssetIntoMemory(Assets::Asset& asset) = 0;


	protected:
		std::unordered_map<AssetHandle, Assets::Asset> m_AssetRegistry{};
		std::unordered_map<AssetHandle, Ref<AssetType>> m_RuntimeCache{};
		std::bitset<8> m_Flags {0b00000001};
	};
}
