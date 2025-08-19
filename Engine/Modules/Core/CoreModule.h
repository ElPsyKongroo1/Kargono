#pragma once

#include "Modules/Core/Module.h"

namespace Kargono
{
	struct AssetTagg {};

	Register_Module(Core)

	struct TestComponent {};

	Register_Module_Type(TestComponent, AssetTagg)
}

