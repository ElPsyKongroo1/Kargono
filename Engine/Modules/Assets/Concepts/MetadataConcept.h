#pragma once
#include "Modules/Core/Concepts/Serializable.h"

namespace Kargono::Assets
{
	template <typename t_Type>
	concept MetadataConcept = HasSerialization<t_Type>;

	template <typename t_Type>
	concept HasMetadata = requires
	{
		typename t_Type::Metadata;
	}&& MetadataConcept<typename t_Type::Metadata>;
}