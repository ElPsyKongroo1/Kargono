#pragma once

#include "Kargono/Core/Base.h"
#include "Modules/Assets/Metadata.h"
#include "Modules/Assets/AssetReference.h"
#include "Modules/Assets/Concepts/SpecificationConcept.h"
#include "Kargono/Core/FixedBufferString.h"

#include <filesystem>
#include <span>

namespace Kargono::Assets
{
	template <typename t_AssetType>
	concept HasSaveValidation = requires (t_AssetType& type, AssetReference<t_AssetType> newAssetRef, Metadata& metadata)
	{
		{ type.SaveValidation(newAssetRef, metadata) } -> std::same_as<Ref<void>>;
	};

	template <typename t_AssetType>
	concept HasDeleteValidation = requires (t_AssetType& type, Metadata& metadata)
	{
		{ type.DeleteValidation(metadata) } -> std::same_as<void>;
	};

	template<typename t_AssetType>
	concept HasFileLocation = requires ()
	{
		{ t_AssetType::GetFileExtension() } -> std::same_as<FixedBufStr16>;
	};

	template<typename t_AssetType>
	concept HasIntermediates = requires ()
	{
		{ t_AssetType::GetIntermediateExtensions() } -> std::same_as<std::span<FixedBufStr16>>;
	};

	template <typename t_AssetType>
	concept HasCreationFromName = requires (Metadata& metadata, std::string_view assetName, std::filesystem::path& assetPath)
	{
		{ t_AssetType::CreateAssetFromName(metadata, assetName, assetPath) } -> std::same_as<void>;
	};

	template<typename t_AssetType>
	concept HasCreationFromFile = requires (Metadata& metadata, std::filesystem::path& sourcePath, 
		std::filesystem::path& assetPath)
	{
		{ t_AssetType::CreateAssetFromFile(metadata, sourcePath, assetPath) } -> std::same_as<void>;
		{ t_AssetType::GetImportExtensions() } -> std::same_as<std::span<FixedBufStr16>>;
	};

	template<typename t_AssetType>
	concept HasCreationFromSpec = HasSpecification<t_AssetType> && requires (Metadata& metadata, 
		typename t_AssetType::Spec& spec, std::filesystem::path& assetPath)
	{
		{ t_AssetType::CreateAssetFromSpec(metadata, spec, assetPath) } -> std::same_as<void>;
	};

	template<typename t_AssetType>
	concept HasSpecValidation = HasCreationFromSpec<t_AssetType> && requires (typename t_AssetType::Spec& spec)
	{
		{ t_AssetType::CreateSpecValidation(spec) } -> std::same_as<bool>;
	};
}