#include "kgpch.h"

#include "Kargono/Core/AppTick.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Utility/Time.h"


namespace Kargono
{
	std::vector<AppTickGenerator> AppTickService::s_AppTickGenerators;
	Events::EventCallbackFn AppTickService::s_AppTickCallback;

	void AppTickService::SetAppTickEventCallback(const Events::EventCallbackFn& callback)
	{
		s_AppTickCallback = callback;
	}

	void AppTickService::OnUpdate(Timestep ts)
	{
		for (AppTickGenerator& generator : s_AppTickGenerators)
		{
			generator.Accumulator += ts;

			if (generator.Accumulator > generator.DelaySeconds)
			{
				generator.Accumulator -= generator.DelaySeconds;
				Events::AppTickEvent event = Events::AppTickEvent(generator.DelayMilliSeconds);
				s_AppTickCallback(&event);
			}
		}
	}

	void AppTickService::ClearGenerators()
	{
		s_AppTickGenerators.clear();
	}

	void AppTickService::ResetAllAccumulators()
	{
		for (AppTickGenerator& generator : s_AppTickGenerators)
		{
			generator.Accumulator = 0;
		}
	}

	void AppTickService::AddNewGenerator(uint64_t delayMilliseconds)
	{
		// Check if a similar generator already exists
		for (AppTickGenerator& generator : s_AppTickGenerators)
		{
			if (delayMilliseconds == generator.DelayMilliSeconds)
			{
				KG_INFO("Attempt to add an AppTickGenerator that already exists!");
				generator.UsageCount++;
				return;
			}
		}

		// Add new generator
		AppTickGenerator newGenerator{};
		newGenerator.Accumulator = 0.0f;
		newGenerator.DelayMilliSeconds = delayMilliseconds;
		newGenerator.DelaySeconds = static_cast<double>(delayMilliseconds) / 1000.0;
		newGenerator.UsageCount = 1;
		s_AppTickGenerators.push_back(newGenerator);
	}

	void AppTickService::RemoveGenerator(uint64_t delayMilliseconds)
	{
		// Decrease usage count if there is a match
		for (AppTickGenerator& generator : s_AppTickGenerators)
		{
			if (delayMilliseconds == generator.DelayMilliSeconds)
			{
				KG_INFO("Attempt to add an AppTickGenerator that already exists!");
				generator.UsageCount--;
			}
		}

		// Remove generators that have usagecount reduced to 0
		auto iter = std::remove_if(s_AppTickGenerators.begin(), s_AppTickGenerators.end(), [&](AppTickGenerator& generator)
		{
			if (generator.UsageCount <= 0)
			{
				return true;
			}

			return false;
		});

		if (iter != s_AppTickGenerators.end())
		{
			s_AppTickGenerators.erase(iter, s_AppTickGenerators.end());
		}
	}

}
