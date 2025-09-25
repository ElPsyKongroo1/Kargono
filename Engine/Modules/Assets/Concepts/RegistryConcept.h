#pragma once

#include "Modules/Core/Concepts/Serializable.h"

namespace Kargono::Assets
{
	template <typename t_Type>
	concept RegistryDataConcept = HasSerialization<t_Type>;

	template <typename t_Type>
	concept HasRegistryData = requires
	{
		typename t_Type::RegistryData;
	}&& RegistryDataConcept<typename t_Type::RegistryData>;
}