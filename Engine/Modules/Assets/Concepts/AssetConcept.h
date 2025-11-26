#pragma once

#include "Modules/Core/Concepts/HasSerialization.h"
#include "Modules/Core/Concepts/HasDisplayName.h"
#include "Modules/Assets/AssetsCommon.h"

#include <concepts>
#include <type_traits>

namespace Kargono::Assets
{
	template <typename t_AssetType>
	concept HasAssetConfig = requires
	{
		{ t_AssetType::GetAssetFlags() } -> std::same_as<AssetFlags>;
	};

	template <typename t_Type>
	concept AssetConcept = HasAssetConfig<t_Type> && 
		HasSerialization<t_Type> && HasDisplayName<t_Type>;
}