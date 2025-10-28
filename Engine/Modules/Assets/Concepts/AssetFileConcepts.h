#pragma once

#include "Kargono/Core/FixedBufferString.h"

#include <concepts>
#include <span>

namespace Kargono::Assets
{
	template<typename t_AssetType>
	concept HasFileLocation = requires ()
	{
		{ t_AssetType::GetFileExtension() } -> std::same_as<FixedBufStr16>;
	};

	template<typename t_AssetType>
	concept HasIntermediates = requires ()
	{
		{ t_AssetType::GetIntermediateExtensions() } -> std::same_as<std::span<const FixedBufStr16>>;
	};
}