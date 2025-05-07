#include "kgpch.h"
#include "Kargono/Core/Base.h"
#include "Kargono/Core/Log.h"
#include "EventModule/ApplicationEvent.h"
#include "Kargono/Core/Engine.h"

#include "API/Logger/SpdlogBackend.h"

namespace Kargono
{
	Ref<spdlog::logger> Log::s_CoreLogger;

	void Log::Init ()
	{
		std::vector<spdlog::sink_ptr> logSinks;
		if (s_TestingActive)
		{
			logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Log/Kargono.log", true));
			logSinks[0]->set_pattern("[%r] | [%s] | [%#] | [%!] | [%n] | [%v]");
		}
		else
		{
			logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
			logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("Log/Kargono.log", true));
			logSinks[0]->set_pattern("%^[%r] [%s:%#]: %v%$");
			logSinks[1]->set_pattern("[%r] | [%s] | [%#] | [%!] | [%n] | [%v]");
		}
		

		s_CoreLogger = std::make_shared<spdlog::logger>("ENGINE", begin(logSinks), end(logSinks));
		spdlog::register_logger(s_CoreLogger);
		s_CoreLogger->set_level(spdlog::level::trace);
		s_CoreLogger->flush_on(spdlog::level::trace);

		spdlog::set_default_logger(s_CoreLogger);

		KG_VERIFY(s_CoreLogger, "Logging System");
	}

	void Log::GenerateLogEventImpl(int logType, const char* text)
	{
		if (EngineService::IsEngineActive() && EngineService::GetActiveEngine().IsApplicationActive())
		{
			if (logType == KG_INFO_LOG_EVENT)
			{
				Events::LogEvent logEvent{ text, Events::LogEventLevel::Info };
				EngineService::GetActiveEngine().GetThread().OnEvent(&logEvent);
			}
			else if (logType == KG_WARNING_LOG_EVENT)
			{
				Events::LogEvent logEvent{ text, Events::LogEventLevel::Warning };
				EngineService::GetActiveEngine().GetThread().OnEvent(&logEvent);
			}
			else if (logType == KG_CRITICAL_LOG_EVENT)
			{
				Events::LogEvent logEvent{ text, Events::LogEventLevel::Critical };
				EngineService::GetActiveEngine().GetThread().OnEvent(&logEvent);
			}
			
		}
	}
}
