#pragma once
#include "API/Logger/SpdLogAPI.h"

namespace Kargono
{
	class Log
	{
	public:
		static void Init();

		static Ref<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }

	private:
		static Ref<spdlog::logger> s_CoreLogger;
	};
}

// General Logging
#ifdef KG_DEBUG
	// Core Log Macros
	#define KG_INFO(...)		SPDLOG_LOGGER_INFO(::Kargono::Log::GetCoreLogger(), __VA_ARGS__)
	#define KG_WARN(...)		SPDLOG_LOGGER_WARN(::Kargono::Log::GetCoreLogger(), __VA_ARGS__)
	#define KG_CRITICAL(...)	SPDLOG_LOGGER_CRITICAL(::Kargono::Log::GetCoreLogger(), __VA_ARGS__)
	#define KG_ERROR(...)		SPDLOG_LOGGER_ERROR(::Kargono::Log::GetCoreLogger(), __VA_ARGS__); KG_DEBUGBREAK();

	// Used for Debugging
	#define KG_TRACE_INFO(...)		SPDLOG_LOGGER_TRACE(::Kargono::Log::GetCoreLogger(), __VA_ARGS__)
	#define KG_TRACE_CRITICAL(...)		SPDLOG_LOGGER_CRITICAL(::Kargono::Log::GetCoreLogger(), __VA_ARGS__)
	#define KG_TRACE_ERROR(...)		SPDLOG_LOGGER_ERROR(::Kargono::Log::GetCoreLogger(), __VA_ARGS__); KG_DEBUGBREAK();
#else
	// Core Log Macros
	#define KG_INFO(...)		
	#define KG_WARN(...)		SPDLOG_LOGGER_WARN(::Kargono::Log::GetCoreLogger(), __VA_ARGS__)
	#define KG_CRITICAL(...)	SPDLOG_LOGGER_CRITICAL(::Kargono::Log::GetCoreLogger(), __VA_ARGS__)
	#define KG_ERROR(...)		SPDLOG_LOGGER_ERROR(::Kargono::Log::GetCoreLogger(), __VA_ARGS__); KG_DEBUGBREAK();

	// Used for Debugging
	#define KG_TRACE(...)		
	#define KG_TRACE2(...)		
	#define KG_TRACE3(...)		
#endif

#ifdef KG_ENABLE_VERIFY
	// Verify: Used to ensure systems are working correctly
	#define KG_VERIFY(predicate, ...) \
	if (predicate) { SPDLOG_LOGGER_INFO(::Kargono::Log::GetCoreLogger(), "[Verified] " __VA_ARGS__); } \
	else { SPDLOG_LOGGER_ERROR(::Kargono::Log::GetCoreLogger(), "[Failed to Verify] " __VA_ARGS__); KG_DEBUGBREAK(); }
#endif

