#pragma once

#pragma warning(push, 0)
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
#define KG_INFO(...)		::Kargono::Log::GetCoreLogger()->info(__VA_ARGS__)
#define KG_WARN(...)		::Kargono::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define KG_CRITICAL(...)	::Kargono::Log::GetCoreLogger()->critical(__VA_ARGS__)
#define KG_ERROR(...)		::Kargono::Log::GetCoreLogger()->error(__VA_ARGS__)

// Used for Debugging
#define KG_TRACE(...)		::Kargono::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define KG_TRACE2(...)		::Kargono::Log::GetCoreLogger()->critical(__VA_ARGS__)
#define KG_TRACE3(...)		::Kargono::Log::GetCoreLogger()->error(__VA_ARGS__)
