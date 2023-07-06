#include "Kargono/kgpch.h"
#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"

/// @brief Namespace for the Kargono game engine
namespace Kargono
{

/// @brief Shared pointer to the core logger of the game engine
	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
/// @brief Shared pointer to the client logger of the game engine
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

/// @brief Initializes the log systems of the game engine
	void Log::Init ()
	{
/// @brief Sets the log pattern for the log systems
		spdlog::set_pattern("%^[%T] %n: %v%$");
/// @brief Creates a new stdout_color_mt core logger with the name 'KARGONO'
		s_CoreLogger = spdlog::stdout_color_mt("KARGONO");
/// @brief Sets the logging level of the core logger to 'trace'
		s_CoreLogger->set_level(spdlog::level::trace);
		
/// @brief Creates a new stdout_color_mt client logger with the name 'APP'
		s_ClientLogger = spdlog::stdout_color_mt("APP");
/// @brief Sets the logging level of the client logger to 'trace'
		s_ClientLogger->set_level(spdlog::level::trace);
	}
}