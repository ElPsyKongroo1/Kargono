#pragma once

#include "Kargono/Core/Base.h"
#include "Modules/Assets/Metadata.h"
#include "Modules/Assets/AssetReference.h"

#include <filesystem>

namespace Kargono::Assets
{
	template <typename t_Type>
	concept HasSaveValidation = requires (t_Type& type, AssetReference<t_Type> newAssetRef, Metadata & metadata)
	{
		{ type.SaveValidation(newAssetRef, metadata) } -> std::same_as<Ref<void>>;
	};

	template <typename t_Type>
	concept HasDeletionValidation = requires (t_Type & type, Metadata & metadata)
	{
		{ type.DeleteValidation(metadata) } -> std::same_as<void>;
	};

	template <typename t_Type>
	concept HasCreationFromName = requires (std::string_view assetName, Metadata & metadata, std::filesystem::path & path)
	{
		{ t_Type::CreateAssetFileFromName(assetName, metadata, path) } -> std::same_as<void>;
	};

	template<typename t_Type>
	concept HasCreationFromFile = requires (Metadata & metadata, std::filesystem::path & filePath, std::filesystem::path & intermediatePath)
	{
		{ t_Type::CreateAssetIntermediateFromFile(metadata, filePath, intermediatePath) } -> std::same_as<void>;
	};
}