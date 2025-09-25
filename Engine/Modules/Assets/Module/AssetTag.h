#pragma once

#include "Modules/Assets/Module/AssetsModule.h"
#include "Modules/FileSystem/FileSystem.h"
#include "Modules/Assets/AssetsCommon.h"
#include "Modules/Core/Concepts/Serializable.h"
#include "Modules/Assets/Asset.h"
#include "Kargono/Core/Base.h"

#include "API/Serialization/yamlcppAPI.h"

#include <filesystem>
#include <cstdint>
#include <limits>
#include <array>

namespace Kargono::Assets
{
	// Registry data concept(s)
	template <typename t_Type>
	concept RegistryDataConcept = HasSerialization<t_Type>;

	template <typename t_Type>
	concept HasRegistryData = requires
	{
		typename t_Type::RegistryData;
	} && RegistryDataConcept<typename t_Type::RegistryData>;
	
	// Metadata concept(s)
	template <typename t_Type>
	concept MetadataConcept = HasSerialization<t_Type>;

	template <typename t_Type>
	concept HasMetadata = requires
	{
		typename t_Type::Metadata;
	} && MetadataConcept<typename t_Type::Metadata>;
	
	// Asset config concept(s)
	template <typename t_Type>
	concept HasAssetConfig = requires
	{
		{ t_Type::GetAssetConfig() } -> std::same_as<AssetConfig>;
	};

	template <typename t_Type>
	concept HasSaveValidation = requires (t_Type & type, Metadata& metadata)
	{
		{ type.SaveValidation(metadata) } -> std::same_as<Ref<void>>;
	};

	template <typename t_Type>
	concept HasDeletionValidation = requires (t_Type & type, Metadata& metadata)
	{
		{ type.DeleteValidation(metadata) } -> std::same_as<void>;
	};

	template <typename t_Type>
	concept HasCreationFromName = requires (std::string_view assetName, Metadata& metadata, std::filesystem::path& path)
	{
		{ t_Type::CreateAssetFileFromName(assetName, metadata, path) } -> std::same_as<void>;
	};

	template<typename t_Type>
	concept HasCreationFromFile = requires (Metadata& metadata, std::filesystem::path& filePath, std::filesystem::path& intermediatePath)
	{
		{ t_Type::CreateAssetIntermediateFromFile(metadata, filePath, intermediatePath) } -> std::same_as<void>;
	};

	// Asset concept(s)
	template <typename t_Type>
	concept AssetConcept = HasAssetConfig<t_Type> && HasSerialization<t_Type>;

	Register_Module_Tag(Asset, AssetConcept<t_Type>)

	template<AssetConcept t_AssetType>
	consteval AssetIdentifier GetAssetIdentifier()
	{
		return GetModuleTypeIdentifier<t_AssetType>();
	}

	struct DeserializeAssetContext
	{
		Metadata* m_AssetMetadata{ nullptr };
		std::filesystem::path m_AssetPath;
	};

	struct SerializeAssetContext
	{
		std::filesystem::path m_AssetPath;
	};

	struct DeserializeRegistryContext
	{
		YAML::Node* m_RegistryNode{ nullptr };
	};

	struct SerializeRegistryContext
	{
		YAML::Emitter* m_Serializer{ nullptr };
	};

	struct SerializeMetaDataContext
	{
		YAML::Emitter* m_Serializer{ nullptr };
		Metadata* m_Metadata{ nullptr };
	};

	struct DeserializeMetaDataContext
	{
		YAML::Node* m_Node{ nullptr };
		Metadata* m_Metadata{ nullptr };
	};
}