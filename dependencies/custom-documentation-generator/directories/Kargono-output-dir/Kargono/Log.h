#pragma once
#include "Core.h"
#include <spdlog/spdlog.h>
#include "spdlog/fmt/ostr.h"


/// @namespace Kargono
/// @brief Namespace for the game engine
namespace Kargono
{
/// @class Log
/// @brief Class for logging in the game engine
	class KG_API Log
	{
	public:
/// @fn void Kargono::Log::Init()
/// @brief Initializes the logging system
		static void Init();

/// @fn std::shared_ptr<spdlog::logger>& Kargono::Log::GetCoreLogger()
/// @brief Returns the core logger
		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
/// @fn std::shared_ptr<spdlog::logger>& Kargono::Log::GetClientLogger()
/// @brief Returns the client logger
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

// Core Log Macros
#define KG_CORE_ERROR(...)	::Kargono::Log::GetCoreLogger()->error(__VA_ARGS__)
#define KG_CORE_WARN(...)	::Kargono::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define KG_CORE_INFO(...)	::Kargono::Log::GetCoreLogger()->info(__VA_ARGS__)
#define KG_CORE_TRACE(...)	::Kargono::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define KG_CORE_FATAL(...)	::Kargono::Log::GetCoreLogger()->fatal(__VA_ARGS__)

// Client Log Macros
/// @def KG_ERROR
/// @brief Macro for logging client error messages
#define KG_ERROR(...) ::Kargono::Log::GetClientLogger()->error(__VA_ARGS__)
/// @def KG_WARN
/// @brief Macro for logging client warning messages
#define KG_WARN(...) ::Kargono::Log::GetClientLogger()->warn(__VA_ARGS__)
/// @def KG_INFO
/// @brief Macro for logging client info messages
#define KG_INFO(...) ::Kargono::Log::GetClientLogger()->info(__VA_ARGS__)
/// @def KG_TRACE
/// @brief Macro for logging client trace messages
#define KG_TRACE(...) ::Kargono::Log::GetClientLogger()->trace(__VA_ARGS__)
/// @def KG_FATAL
/// @brief Macro for logging client fatal messages
#define KG_FATAL(...) ::Kargono::Log::GetClientLogger()->fatal(__VA_ARGS__)

