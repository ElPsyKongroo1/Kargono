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
	template <typename t_Type>
	concept HasSaveValidation = requires (t_Type& type, AssetReference<t_Type> newAssetRef, Metadata& metadata)
	{
		{ type.SaveValidation(newAssetRef, metadata) } -> std::same_as<Ref<void>>;
	};

	template <typename t_Type>
	concept HasDeleteValidation = requires (t_Type& type, Metadata & metadata)
	{
		{ type.DeleteValidation(metadata) } -> std::same_as<void>;
	};

	template <typename t_Type>
	concept HasCreationFromName = requires (Metadata& metadata, std::string_view assetName, std::filesystem::path& assetPath)
	{
		{ t_Type::CreateAssetFromName(metadata, assetName, assetPath) } -> std::same_as<void>;
	};

	template<typename t_Type>
	concept HasCreationFromFile = requires (Metadata& metadata, std::filesystem::path& sourcePath, 
		std::filesystem::path& assetPath)
	{
		{ t_Type::CreateAssetFromFile(metadata, sourcePath, assetPath) } -> std::same_as<void>;
		{ t_Type::GetImportExtensions() } -> std::same_as<std::span<FixedBufStr16>>;
	};

	template<typename t_Type>
	concept HasCreationFromSpec = HasSpecification<t_Type> && requires (Metadata& metadata, 
		typename t_Type::Spec& spec, std::filesystem::path& assetPath)
	{
		{ t_Type::CreateAssetFromSpec(metadata, spec, assetPath) } -> std::same_as<void>;
	};

	template<typename t_Type>
	concept HasSpecValidation = HasCreationFromSpec<t_Type> && requires (typename t_Type::Spec& spec)
	{
		{ t_Type::CreateSpecValidation(spec) } -> std::same_as<bool>;
	};
}