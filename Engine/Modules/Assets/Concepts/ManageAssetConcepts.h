#pragma once

#include "Modules/Assets/Metadata.h"
#include "Modules/Assets/Concepts/SpecificationConcept.h"
#include "Modules/Assets/Concepts/AssetFileConcepts.h"
#include "Modules/Assets/Concepts/AssetConcept.h"
#include "Modules/Assets/Concepts/AssetFlagConcepts.h"
#include "Modules/Assets/AssetReference.h"
#include "Modules/FileSystem/FileSystem.h"

#include <concepts>
#include <filesystem>
#include <span>

namespace Kargono::Assets
{
	// Asset update
	template <typename t_AssetType>
	concept HasDefaultUpdateFromAsset = HasAssetCacheFlag<t_AssetType> && 
		HasAllowDefaultUpdateAssetFlag<t_AssetType>;

	template <typename t_AssetType>
	concept HasCustomUpdateFromAsset = HasAssetCacheFlag<t_AssetType> && 
		requires (Metadata & metadata, t_AssetType & current, AssetReference<t_AssetType> otherRef)
	{
		{ current.UpdateFromAsset(metadata, otherRef) } -> std::same_as<void>;
	};

	template <typename t_AssetType>
	concept HasUpdateFromAsset = HasDefaultUpdateFromAsset<t_AssetType> || HasCustomUpdateFromAsset<t_AssetType>;

	template <typename t_AssetType>
	concept HasValidateUpdateFromAsset = HasUpdateFromAsset<t_AssetType> && 
		requires (Metadata& metadata, AssetReference<t_AssetType> newAssetRef)
	{
		{ t_AssetType::ValidateUpdateFromAsset(metadata, newAssetRef) } -> std::same_as<Ref<void>>;
	};

	template <typename t_AssetType>
	concept HasUpdateFromSpec = HasAssetCacheFlag<t_AssetType> &&
		HasSpecification<t_AssetType> && 
		requires (Metadata& metadata, t_AssetType& current, const typename t_AssetType::Spec& spec)
	{
		{ current.UpdateFromSpec(metadata, spec) } -> std::same_as<void>;
	};

	template <typename t_AssetType>
	concept HasValidateUpdateFromSpec = HasUpdateFromSpec<t_AssetType> &&
		requires (Metadata & metadata, const typename t_AssetType::Spec & spec)
	{
		{ t_AssetType::ValidateUpdateFromSpec(metadata, spec) } -> std::same_as<Ref<void>>;
	};

	// Asset delete
	template <typename t_AssetType>
	concept HasValidateDelete = requires (Metadata& metadata)
	{
		{ t_AssetType::ValidateDelete(metadata) } -> std::same_as<void>;
	};

	// Asset create
	template <typename t_AssetType>
	concept HasCreateFromName = requires (Metadata& metadata)
	{
		{ t_AssetType::CreateFromName(metadata) } -> std::same_as<void>;
	};

	template<typename t_AssetType>
	concept HasCreateFromFile = HasImportExtensions<t_AssetType> && requires (
		Metadata & metadata, std::filesystem::path& sourcePath)
	{
		{ t_AssetType::CreateFromFile(metadata, sourcePath) } -> std::same_as<void>;
	};

	template<typename t_AssetType>
	concept HasCreateFromSpec = HasSpecification<t_AssetType> && requires (Metadata& metadata,
		const typename t_AssetType::Spec& spec)
	{
		{ t_AssetType::CreateFromSpec(metadata, spec) } -> std::same_as<void>;
	};

	template<typename t_AssetType>
	concept HasValidateCreateFromSpec = HasCreateFromSpec<t_AssetType> && requires (
		const AssetCreationData& creationData, const typename t_AssetType::Spec& spec)
	{
		{ t_AssetType::ValidateCreateFromSpec(creationData, spec) } -> std::same_as<bool>;
	};

	// Get asset
	template<typename t_AssetType>
	concept HasGetAssetFromSpec = HasSpecification<t_AssetType> && requires (Metadata & metadata,
		const typename t_AssetType::Spec& spec)
	{
		{ t_AssetType::GetAssetFromSpec(metadata, spec) } -> std::same_as<bool>;
	};

	// Generate asset hash from spec
	template<typename t_AssetType>
	concept HasHashFromSpec = HasSpecification<t_AssetType> && requires (const typename t_AssetType::Spec& spec)
	{
		{ t_AssetType::GetHashFromSpec(spec) } -> std::same_as<Utility::SHA256Hash>;
	};

}