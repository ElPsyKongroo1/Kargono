#pragma once

#include "Kargono/Core/Timestep.h"
#include "Kargono/Events/Event.h"
#include "Kargono/Events/ApplicationEvent.h"

#include <vector>

namespace Kargono
{
	//==============================
	// App Tick Generator Struct
	//==============================
	struct AppTickGenerator
	{
		double Accumulator{ 0.0 };
		uint64_t DelayMilliSeconds{ 0 };
		double DelaySeconds{ 0.0 };
		uint16_t UsageCount { 0 };
	};
	//==============================
	// App Tick API Class
	//==============================
	class AppTickService
	{
	public:
		//==============================
		// Load Generators From File
		//==============================
		static void LoadGeneratorsFromProject();

		//==============================
		// Manage Events
		//==============================
		static void SetAppTickEventCallback(const Events::EventCallbackFn& callback);

		//==============================
		// OnEvent Functions
		//==============================
		static void OnUpdate(Timestep ts);

		//==============================
		// Manage Generators
		//==============================
		static void ClearGenerators();
		static void ResetAllAccumulators();
		static void AddNewGenerator(uint64_t delayMilliseconds);
		static void RemoveGenerator(uint64_t delayMilliseconds);
	private:
		//==============================
		// Internal Fields
		//==============================
		static std::vector<AppTickGenerator> s_AppTickGenerators;
		static Events::EventCallbackFn s_AppTickCallback;
	};
}
