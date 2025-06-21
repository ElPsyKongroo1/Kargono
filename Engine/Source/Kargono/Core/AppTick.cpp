#include "kgpch.h"

#include "Kargono/Core/AppTick.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Utility/Time.h"

namespace Kargono
{
	void AppTickContext::SetAppTickEventCallback(const Events::EventCallbackFn& callback)
	{
		s_AppTickCallback = callback;
	}

	void AppTickContext::OnUpdate(Timestep ts)
	{
		for (AppTickGenerator& generator : s_AppTickGenerators)
		{
			generator.m_Accumulator += ts;

			if (generator.m_Accumulator > generator.m_DelaySeconds)
			{
				generator.m_Accumulator -= generator.m_DelaySeconds;
				Events::AppTickEvent event = Events::AppTickEvent(generator.m_DelayMilliSeconds);
				s_AppTickCallback(&event);
			}
		}
	}

	void AppTickContext::ClearGenerators()
	{
		s_AppTickGenerators.clear();
	}

	void AppTickContext::ResetAllAccumulators()
	{
		for (AppTickGenerator& generator : s_AppTickGenerators)
		{
			generator.m_Accumulator = 0;
		}
	}

	void AppTickContext::AddNewGenerator(uint64_t delayMilliseconds)
	{
		// Check if a similar generator already exists
		for (AppTickGenerator& generator : s_AppTickGenerators)
		{
			if (delayMilliseconds == generator.m_DelayMilliSeconds)
			{
				KG_INFO("Attempt to add an AppTickGenerator that already exists!");
				generator.m_UsageCount++;
				return;
			}
		}

		// Add new generator
		AppTickGenerator newGenerator{};
		newGenerator.m_Accumulator = 0.0f;
		newGenerator.m_DelayMilliSeconds = delayMilliseconds;
		newGenerator.m_DelaySeconds = static_cast<double>(delayMilliseconds) / 1000.0;
		newGenerator.m_UsageCount = 1;
		s_AppTickGenerators.push_back(newGenerator);
	}

	void AppTickContext::RemoveGenerator(uint64_t delayMilliseconds)
	{
		// Decrease usage count if there is a match
		for (AppTickGenerator& generator : s_AppTickGenerators)
		{
			if (delayMilliseconds == generator.m_DelayMilliSeconds)
			{
				KG_INFO("Attempt to add an AppTickGenerator that already exists!");
				generator.m_UsageCount--;
			}
		}

		// Remove generators that have usagecount reduced to 0
		auto iter = std::remove_if(s_AppTickGenerators.begin(), s_AppTickGenerators.end(), [&](AppTickGenerator& generator)
		{
			if (generator.m_UsageCount <= 0)
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
