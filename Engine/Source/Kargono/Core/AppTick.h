#pragma once

#include "Kargono/Core/Timestep.h"
#include "Kargono/Events/Event.h"

namespace Kargono
{
	//==============================
	// App Tick Generator Struct
	//==============================
	struct AppTickGenerator
	{
		double Accumulator;
		uint64_t DelayMilliSeconds;
		double DelaySeconds;
		uint16_t UsageCount { 0 };
	};
	//==============================
	// App Tick API Class
	//==============================
	class AppTickEngine
	{
	public:
		//==============================
		// Load From File
		//==============================
		static void LoadProjectGenerators();
		//==============================
		// Event Functions
		//==============================
		static void SetAppTickCallback(const Events::EventCallbackFn& callback);
		static void UpdateGenerators(Timestep ts);
		//==============================
		// Manage Generators
		//==============================
		static void ClearGenerators();
		static void ResetAllAccumulators();
		static void AddGeneratorUsage(uint64_t delayMilliseconds);
		static void RemoveGeneratorUsage(uint64_t delayMilliseconds);
		
	};
}
