#pragma once

#include "Kargono/Core/Timestep.h"
#include "Modules/Events/Event.h"
#include "Modules/Events/ApplicationEvent.h"

#include <vector>

namespace Kargono
{
	struct AppTickGenerator
	{
		double m_Accumulator{ 0.0 };
		uint64_t m_DelayMilliSeconds{ 0 };
		double m_DelaySeconds{ 0.0 };
		uint16_t m_UsageCount { 0 };
	};

	class AppTickContext
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		AppTickContext() = default;
		~AppTickContext() = default;
	public:
		//==============================
		// Manage Events
		//==============================
		void SetAppTickEventCallback(const Events::EventCallbackFn& callback);
	public:
		//==============================
		// OnEvent Functions
		//==============================
		void OnUpdate(Timestep ts);
	public:
		//==============================
		// Manage Generators
		//==============================
		void ClearGenerators();
		void ResetAllAccumulators();
		void AddNewGenerator(uint64_t delayMilliseconds);
		void RemoveGenerator(uint64_t delayMilliseconds);
	private:
		//==============================
		// Internal Fields
		//==============================
		std::vector<AppTickGenerator> s_AppTickGenerators;
		Events::EventCallbackFn s_AppTickCallback;
	};

	class AppTickService // TODO: EWWWWW UGHHHHHHH
	{
	public:
		//==============================
		// Getters/Setters
		//==============================
		static AppTickContext& GetActiveContext() { return s_AppTickContext; }
	private:
		//==============================
		// Internal Fields
		//==============================
		static inline AppTickContext s_AppTickContext{};
	};
}
