#pragma once
#include <concepts>

namespace Kargono
{
	template <typename t_Type>
	concept HasSerialization = requires (t_Type type, void* context)
	{
		{ type.Serialize(context) } -> std::same_as<void>;
		{ type.Deserialize(context) } -> std::same_as<void>;
	};
}