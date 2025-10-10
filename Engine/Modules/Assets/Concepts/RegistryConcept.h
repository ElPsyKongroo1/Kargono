#pragma once

#include "Modules/Core/Concepts/Serializable.h"

namespace Kargono::Assets
{
	template <typename t_RegistryDataType>
	concept RegistryDataConcept = HasSerialization<t_RegistryDataType>;

	template <typename t_AssetType>
	concept HasRegistryData = requires
	{
		typename t_AssetType::RegistryData;
	} && RegistryDataConcept<typename t_AssetType::RegistryData>;
}