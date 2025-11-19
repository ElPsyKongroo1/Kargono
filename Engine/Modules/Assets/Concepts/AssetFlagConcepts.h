#pragma once

#include "Modules/Assets/Concepts/AssetConcept.h"
#include "Modules/Assets/AssetsCommon.h"

namespace Kargono::Assets
{
	// Generic asset flag check
	template<typename t_AssetType, AssetFlag flag>
	concept HasAssetFlag = AssetConcept<t_AssetType> && t_AssetType::GetAssetFlags().IsFlagSet(flag);

	// Preset flag checks
	template<typename t_AssetType>
	concept HasAssetCacheFlag = HasAssetFlag<t_AssetType, AssetFlag::HasAssetCache>;

	template<typename t_AssetType>
	concept HasRequireUniqueNameFlag = HasAssetFlag<t_AssetType, AssetFlag::RequireUniqueName>;

	template<typename t_AssetType>
	concept HasRequireUniqueHashFlag = HasAssetFlag<t_AssetType, AssetFlag::RequireUniqueHash>;

}