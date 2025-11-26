#pragma once
#include "Kargono/Core/FixedBufferString.h"

namespace Kargono
{
	template<typename t_Type>
	concept HasDisplayName = requires
	{
		{ t_Type::GetDisplayName() } -> std::same_as<FixedBufStr32>;
	};
}