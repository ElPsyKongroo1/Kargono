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
		uint16_t UsageCount { 0 };
	};

	class AppTickEngine
	{
	public:
		static void LoadProjectGenerators();
		static void SetAppTickCallback(const Events::EventCallbackFn& callback);
		static void UpdateGenerators(Timestep ts);
		static void ClearGenerators();
		static void ResetAllAccumulators();
		static void AddGeneratorUsage(uint64_t delayMilliseconds);
		static void RemoveGeneratorUsage(uint64_t delayMilliseconds);
		
	};
}
