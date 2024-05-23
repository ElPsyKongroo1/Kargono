#pragma once

#pragma warning(push, 0)
#ifdef KG_DEBUG
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#endif
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)


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

// Core Log Macros
#define KG_INFO(...)		SPDLOG_LOGGER_INFO(::Kargono::Log::GetCoreLogger(), __VA_ARGS__)
#define KG_WARN(...)		SPDLOG_LOGGER_WARN(::Kargono::Log::GetCoreLogger(), __VA_ARGS__)
#define KG_CRITICAL(...)	SPDLOG_LOGGER_CRITICAL(::Kargono::Log::GetCoreLogger(), __VA_ARGS__)
#define KG_ERROR(...)		SPDLOG_LOGGER_ERROR(::Kargono::Log::GetCoreLogger(), __VA_ARGS__)

// Used for Debugging
#define KG_TRACE(...)		SPDLOG_LOGGER_TRACE(::Kargono::Log::GetCoreLogger(), __VA_ARGS__)
#define KG_TRACE2(...)		SPDLOG_LOGGER_CRITICAL(::Kargono::Log::GetCoreLogger(), __VA_ARGS__)
#define KG_TRACE3(...)		SPDLOG_LOGGER_ERROR(::Kargono::Log::GetCoreLogger(), __VA_ARGS__)

