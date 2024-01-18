#pragma once

#include "Kargono/Core/Timestep.h"

#include <vector>

#include "Kargono/Events/Event.h"

namespace Kargono
{
	struct AppTickGenerator
	{
		double Accumulator;
		uint64_t DelayMilliSeconds;
		double DelaySeconds;
	};

	class AppTickEngine
	{
	public:
		static void LoadProjectGenerators();
		static void SetAppTickCallback(const Events::EventCallbackFn& callback);
		static void UpdateGenerators(Timestep ts);
		static void ClearGenerators();
		static void ResetAllAccumulators();
		static void AddGenerator(uint64_t delayMilliseconds);
		
	};
}
