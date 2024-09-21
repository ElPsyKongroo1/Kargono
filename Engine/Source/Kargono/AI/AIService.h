#pragma once
#include "Kargono/Core/Timestep.h"

namespace Kargono::AI
{
	struct AIState
	{
		// Hold script for OnUpdate?
	};

	class AIService
	{
	public:
		static void OnUpdate(Timestep ts);
	};
}
