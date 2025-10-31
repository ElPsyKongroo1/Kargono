#pragma once

#include "Modules/Assets/Metadata.h"
#include "Modules/Assets/Concepts/SpecificationConcept.h"
#include "Modules/Assets/Concepts/AssetFileConcepts.h"
#include "Modules/Assets/Concepts/AssetConcept.h"
#include "Modules/Assets/AssetReference.h"

#include <concepts>
#include <filesystem>
#include <span>

namespace Kargono::Assets
{
	template<typename t_AssetType>
	concept HasAssetSaving = HasFileLocation<t_AssetType> || HasIntermediates<t_AssetType>;

	template <typename t_AssetType>
	concept HasSaveValidation = requires (t_AssetType & type, AssetReference<t_AssetType> newAssetRef, Metadata & metadata)
	{
		{ type.SaveValidation(newAssetRef, metadata) } -> std::same_as<Ref<void>>;
	};

	template <typename t_AssetType>
	concept HasDeleteValidation = requires (t_AssetType & type, Metadata& metadata)
	{
		{ type.DeleteValidation(metadata) } -> std::same_as<void>;
	};

	template <typename t_AssetType>
	concept HasCreationFromName = requires (Metadata & metadata)
	{
		{ t_AssetType::CreateAssetFromName(metadata) } -> std::same_as<void>;
	};

	template<typename t_AssetType>
	concept HasCreationFromFile = requires (Metadata & metadata, std::filesystem::path & sourcePath)
	{
		{ t_AssetType::CreateAssetFromFile(metadata, sourcePath) } -> std::same_as<void>;
		{ t_AssetType::GetImportExtensions() } -> std::same_as<std::span<const FixedBufStr16>>;
	};

	template<typename t_AssetType>
	concept HasCreationFromSpec = HasSpecification<t_AssetType> && requires (Metadata& metadata,
		typename t_AssetType::Spec & spec)
	{
		{ t_AssetType::CreateAssetFromSpec(metadata, spec) } -> std::same_as<void>;
	};

	template<typename t_AssetType>
	concept HasSpecValidation = HasCreationFromSpec<t_AssetType> && requires (typename t_AssetType::Spec & spec)
	{
		{ t_AssetType::CreateSpecValidation(spec) } -> std::same_as<bool>;
	};
}