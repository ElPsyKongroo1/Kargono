#include "Kargono/kgpch.h"
#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace Kargono
{

	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

/// @brief Initializes the logging system in the game engine

/// @details Sets the pattern for log messages, creates logger instances
/// for both the core and client application, and sets the logging level for both.
	void Log::Init ()
	{
/// @brief Sets the pattern for log messages

/// @param pattern A string representing the log message pattern
		spdlog::set_pattern("%^[%T] %n: %v%$");
/// @brief Creates a logger instance for the core application

/// @param loggerName The name of the logger
		s_CoreLogger = spdlog::stdout_color_mt("KARGONO");
/// @brief Sets the logging level for the core application

/// @param level The logging level to be set
		s_CoreLogger->set_level(spdlog::level::trace);
		
/// @brief Creates a logger instance for the client application

/// @param loggerName The name of the logger
		s_ClientLogger = spdlog::stdout_color_mt("APP");
/// @brief Sets the logging level for the client application

/// @param level The logging level to be set
		s_ClientLogger->set_level(spdlog::level::trace);
	}
}