#include "kgpch.h"

#include "Kargono/Core/AppTick.h"

#include "Kargono/Events/ApplicationEvent.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Utils/PlatformUtils.h"


namespace Kargono
{
	static std::vector<AppTickGenerator> s_AppTickGenerators {};
	static Events::EventCallbackFn s_AppTickCallback {};

	void AppTickEngine::LoadProjectGenerators()
	{
		AppTickEngine::ClearGenerators();
		for (auto generatorValue : Projects::Project::GetAppTickGenerators())
		{
			AppTickEngine::AddGenerator(generatorValue);
		}
	}

	void AppTickEngine::SetAppTickCallback(const Events::EventCallbackFn& callback)
	{
		s_AppTickCallback = callback;
	}

	void AppTickEngine::UpdateGenerators(Timestep ts)
	{
		for (auto& generator : s_AppTickGenerators)
		{
			generator.Accumulator += ts;

			if (generator.Accumulator > generator.DelaySeconds)
			{
				generator.Accumulator -= generator.DelaySeconds;
				Events::AppTickEvent event = Events::AppTickEvent(generator.DelayMilliSeconds);
				s_AppTickCallback(event);
				KG_CORE_INFO("An App Tick has been generated with a delay of {}", Utility::Time::GetStringFromMilliseconds(generator.DelayMilliSeconds));
			}
		}
	}

	void AppTickEngine::ClearGenerators()
	{
		s_AppTickGenerators.clear();
	}

	void AppTickEngine::ResetAllAccumulators()
	{
		for (auto& generator : s_AppTickGenerators)
		{
			generator.Accumulator = 0;
		}
	}

	void AppTickEngine::AddGenerator(uint64_t delayMilliseconds)
	{
		// Check if a similar generator already exists
		for (auto& generator : s_AppTickGenerators)
		{
			if (delayMilliseconds == generator.DelayMilliSeconds)
			{
				KG_CORE_INFO("Attempt to add an AppTickGenerator that already exists!");
				return;
			}
		}

		// Add new generator
		AppTickGenerator newGenerator{};
		newGenerator.Accumulator = 0.0f;
		newGenerator.DelayMilliSeconds = delayMilliseconds;
		newGenerator.DelaySeconds = static_cast<double>(delayMilliseconds) / 1000.0;
		s_AppTickGenerators.push_back(newGenerator);
	}

}
