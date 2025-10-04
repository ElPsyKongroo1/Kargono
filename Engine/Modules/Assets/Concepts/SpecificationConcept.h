#pragma once

namespace Kargono::Assets
{
	template <typename t_Type>
	concept HasSpecification = requires
	{
		typename t_Type::Spec;
	};
}