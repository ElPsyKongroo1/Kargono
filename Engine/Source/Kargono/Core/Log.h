#pragma once
#include "API/Logger/SpdLogAPI.h"

#define KG_STRINGIFY(x) #x
#define KG_EXPAND_AND_STRINGIFY(...) KG_STRINGIFY(__VA_ARGS__)

namespace Kargono
{
	class Log
	{
	public:
		static void Init();
		static void GenerateWarningEvent(const char* format);
		static void GenerateWarningEvent(const char* format, fmt::format_args args);
		template <typename... Args>
		static void GenerateWarningEvent(const char* format, Args&&... args) 
		{
			return GenerateWarningEvent(format, fmt::make_format_args(std::forward<Args>(args)...));
		}

		static Ref<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }

	private:
		static Ref<spdlog::logger> s_CoreLogger;
	};
}

// General Logging
#if defined(KG_DEBUG) || defined(KG_EXPORT_SERVER)
	// Core Log Macros
	#define KG_INFO(...)		SPDLOG_LOGGER_INFO(::Kargono::Log::GetCoreLogger(), __VA_ARGS__)
	#define KG_WARN(...)		SPDLOG_LOGGER_WARN(::Kargono::Log::GetCoreLogger(), __VA_ARGS__)
	#define KG_CRITICAL(...)	SPDLOG_LOGGER_CRITICAL(::Kargono::Log::GetCoreLogger(), __VA_ARGS__)
	#define KG_ERROR(...)		SPDLOG_LOGGER_ERROR(::Kargono::Log::GetCoreLogger(), __VA_ARGS__); KG_DEBUGBREAK_MSG("[KG_ERROR]: " KG_EXPAND_AND_STRINGIFY(__VA_ARGS__));

	// Used for Debugging
	#define KG_TRACE_INFO(...)		SPDLOG_LOGGER_TRACE(::Kargono::Log::GetCoreLogger(), __VA_ARGS__)
	#define KG_TRACE_CRITICAL(...)		SPDLOG_LOGGER_CRITICAL(::Kargono::Log::GetCoreLogger(), __VA_ARGS__)
	#define KG_TRACE_ERROR(...)		SPDLOG_LOGGER_ERROR(::Kargono::Log::GetCoreLogger(), __VA_ARGS__); KG_DEBUGBREAK_MSG("[KG_ERROR]: " KG_EXPAND_AND_STRINGIFY(__VA_ARGS__));
#else
	// Core Log Macros
	#define KG_INFO(...)		
	#define KG_WARN(...)		SPDLOG_LOGGER_WARN(::Kargono::Log::GetCoreLogger(), __VA_ARGS__)
	#define KG_CRITICAL(...)	SPDLOG_LOGGER_CRITICAL(::Kargono::Log::GetCoreLogger(), __VA_ARGS__)
	#define KG_ERROR(...)		SPDLOG_LOGGER_ERROR(::Kargono::Log::GetCoreLogger(), __VA_ARGS__); KG_DEBUGBREAK_MSG("[KG_ERROR]: " KG_EXPAND_AND_STRINGIFY(__VA_ARGS__));

	// Used for Debugging
	#define KG_TRACE_INFO(...)		
	#define KG_TRACE_CRITICAL(...)		
	#define KG_TRACE_ERROR(...)		
#endif

#ifdef KG_ENABLE_VERIFY
	// Verify: Used to ensure systems are working correctly
	#define KG_VERIFY(predicate, ...) \
	if (predicate) { SPDLOG_LOGGER_INFO(::Kargono::Log::GetCoreLogger(), "[Verified] " __VA_ARGS__); } \
	else { SPDLOG_LOGGER_ERROR(::Kargono::Log::GetCoreLogger(), "[Failed to Verify] " __VA_ARGS__); KG_DEBUGBREAK_MSG("[Failed to Verify]: " KG_EXPAND_AND_STRINGIFY(__VA_ARGS__));  }
#endif

