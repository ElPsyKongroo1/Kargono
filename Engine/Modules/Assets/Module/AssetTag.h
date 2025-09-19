#pragma once

#include "Modules/Assets/Module/AssetsModule.h"
#include "Modules/FileSystem/FileSystem.h"
#include "Modules/Assets/AssetsCommon.h"
#include "Modules/Core/Concepts/Serializable.h"

#include <filesystem>
#include <cstdint>
#include <limits>
#include <array>

namespace Kargono::Assets
{
	// Assets
	template <typename t_Type>
	concept HasAssetConfig = requires ()
	{
		{ t_Type::GetAssetConfig() } -> std::same_as<AssetConfig>;
	};

	template <typename t_Type>
	concept MetadataConcept = HasSerialization<t_Type>;

	template<typename t_Type>
	concept HasRegistrySerialization = requires (t_Type asset, void* context)
	{
		{ t_Type::DeserializeRegistryData(context) } -> std::same_as<void>;
		{ t_Type::SerializeRegistryData(context) } -> std::same_as<void>;
	};

	template <typename t_Type>
	concept HasAssetMetadata = requires (t_Type asset)
	{
		{ asset.GetAssetMetadata() } -> MetadataConcept;
		{ asset.SetAssetMetadata(std::declval<decltype(asset.GetAssetMetadata())>()) } -> std::same_as<void>;
	};

	template <typename t_Type>
	concept AssetConcept = HasAssetConfig<t_Type> && HasSerialization<t_Type>;

	Register_Module_Tag(Asset, AssetConcept<t_Type>)

	template<AssetConcept t_AssetType>
	consteval AssetIdentifier GetAssetIdentifier()
	{
		return GetModuleTypeIdentifier<t_AssetType>();
	}
}