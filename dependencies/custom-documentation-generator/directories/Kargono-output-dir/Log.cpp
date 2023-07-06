#include "Kargono/kgpch.h"
#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Kargono
{

	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

	void Log::Init ()
	{
/// @param pattern The logging pattern to be used
		spdlog::set_pattern("%^[%T] %n: %v%$");
/// @brief Create a new console logger for the core engine
		s_CoreLogger = spdlog::stdout_color_mt("KARGONO");
/// @param level The log level to be set for the core engine logger
		s_CoreLogger->set_level(spdlog::level::trace);
		
/// @brief Create a new console logger for the client application
		s_ClientLogger = spdlog::stdout_color_mt("APP");
/// @param level The log level to be set for the client application logger
		s_ClientLogger->set_level(spdlog::level::trace);
	}
}