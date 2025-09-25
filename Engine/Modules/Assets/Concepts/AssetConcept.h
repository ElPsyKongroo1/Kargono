#pragma once

#include "Modules/Core/Concepts/Serializable.h"
#include "Modules/Assets/AssetsCommon.h"

namespace Kargono::Assets
{
	template <typename t_Type>
	concept HasAssetConfig = requires
	{
		{ t_Type::GetAssetConfig() } -> std::same_as<AssetConfig>;
	};

	template <typename t_Type>
	concept AssetConcept = HasAssetConfig<t_Type> && HasSerialization<t_Type>;
}