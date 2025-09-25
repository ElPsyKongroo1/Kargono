#pragma once

#include "Modules/Assets/Module/AssetsModule.h"
#include "Modules/FileSystem/FileSystem.h"
#include "Modules/Assets/AssetsCommon.h"
#include "Modules/Assets/Metadata.h"
#include "Modules/Assets/Concepts/AssetConcept.h"
#include "Kargono/Core/Base.h"

#include "API/Serialization/yamlcppAPI.h"

#include <filesystem>
#include <cstdint>
#include <limits>
#include <array>

namespace Kargono::Assets
{
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