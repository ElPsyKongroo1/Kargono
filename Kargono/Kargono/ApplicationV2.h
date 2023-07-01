#pragma once

#include "Core.h"

namespace Karg
{
	class KG_API ApplicationV2
	{
	public:
		ApplicationV2();
		virtual ~ApplicationV2();
		void Run();
	};

	// To be defined in client
	ApplicationV2* CreateApplication();
}

