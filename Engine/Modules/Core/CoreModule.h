#pragma once

#include "Modules/Core/Module.h"

namespace Kargono
{
	struct AssetTag
	{
		template<typename t_QueryType>
		consteval static bool Check()
		{
			return true;
		}
	};

	Register_Module(Core)

	struct TestComponent {};

	Register_Module_Type(TestComponent, AssetTag)
}

