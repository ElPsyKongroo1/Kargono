#pragma once

#include "Core.h"
#include "Events/Event.h"

namespace Kargono
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

