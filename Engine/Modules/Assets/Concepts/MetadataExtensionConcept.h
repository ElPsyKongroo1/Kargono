#pragma once
#include "Modules/Core/Concepts/HasSerialization.h"

#include <type_traits>

namespace Kargono::Assets
{
	// Requirements of a metadata extension
	template <typename t_MetadataExtension>
	concept MetadataExtensionConcept = HasSerialization<t_MetadataExtension>;

	// Check if an asset type has a metadata extension
	template <typename t_AssetType>
	concept HasMetadataExtension = requires
	{
		typename t_AssetType::MetadataExtension;
	} && MetadataExtensionConcept<typename t_AssetType::MetadataExtension>;
}

namespace Kargono::Detail
{
	template<typename t_AssetType>
	consteval auto GetMetadataExtensionType()
	{
		// Retrieve metadata extension type or std::monostate if none exists
		if constexpr (Assets::HasMetadataExtension<t_AssetType>)
		{
			return std::type_identity<typename t_AssetType::MetadataExtension>{};
		}
		else
		{
			return std::type_identity<std::monostate>{};
		}
	}
}
namespace Kargono::Assets
{
	// Metadata extension type. 
	// Use this type when you need to refer to the metadata extension type directly
	template<typename t_AssetType>
	using MetadataExtension_t = typename decltype(Detail::GetMetadataExtensionType<t_AssetType>())::type;
}
