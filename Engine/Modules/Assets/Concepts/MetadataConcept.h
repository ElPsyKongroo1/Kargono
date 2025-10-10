#pragma once
#include "Modules/Core/Concepts/Serializable.h"

namespace Kargono::Assets
{
	template <typename t_MetadataType>
	concept MetadataConcept = HasSerialization<t_MetadataType>;

	template <typename t_AssetType>
	concept HasMetadata = requires
	{
		typename t_AssetType::Metadata;
	} && MetadataConcept<typename t_AssetType::Metadata>;
}