#pragma once

#include "Modules/Core/Concepts/HasSerialization.h"

namespace Kargono::Assets
{
	// Requirements of a registry extension
	template <typename t_RegistryExtension>
	concept RegistryExtensionConcept = HasSerialization<t_RegistryExtension>;

	// Check if an asset type has a registry extension
	template <typename t_AssetType>
	concept HasRegistryExtension = requires
	{
		typename t_AssetType::RegistryExtension;
	} && RegistryExtensionConcept<typename t_AssetType::RegistryExtension>;
}

namespace Kargono::Detail
{
	template<typename t_AssetType>
	consteval auto GetRegistryExtensionType()
	{
		// Retrieve registry extension type or std::monostate if none exists
		if constexpr (Assets::HasRegistryExtension<t_AssetType>)
		{
			return std::type_identity<typename t_AssetType::RegistryExtension>{};
		}
		else
		{
			return std::type_identity<std::monostate>{};
		}
	}
}
namespace Kargono::Assets
{
	// Registry extension type. 
	// Use this type when you need to refer to the registry extension type directly
	template<typename t_AssetType>
	using RegistryExtension_t = typename decltype(Detail::GetRegistryExtensionType<t_AssetType>())::type;
}