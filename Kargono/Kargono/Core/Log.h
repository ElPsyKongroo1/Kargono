#pragma once
#include "Kargono/Core/Core.h"
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>


namespace Kargono
{
	class Log
	{
	public:
		static void Init();

		static Ref<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		static Ref<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static Ref<spdlog::logger> s_CoreLogger;
		static Ref<spdlog::logger> s_ClientLogger;
	};
}

// Core Log Macros
#define KG_CORE_ERROR(...)	::Kargono::Log::GetCoreLogger()->error(__VA_ARGS__)
#define KG_CORE_WARN(...)	::Kargono::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define KG_CORE_INFO(...)	::Kargono::Log::GetCoreLogger()->info(__VA_ARGS__)
#define KG_CORE_TRACE(...)	::Kargono::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define KG_CORE_CRITICAL(...)	::Kargono::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client Log Macros
#define KG_ERROR(...) ::Kargono::Log::GetClientLogger()->error(__VA_ARGS__)
#define KG_WARN(...) ::Kargono::Log::GetClientLogger()->warn(__VA_ARGS__)
#define KG_INFO(...) ::Kargono::Log::GetClientLogger()->info(__VA_ARGS__)
#define KG_TRACE(...) ::Kargono::Log::GetClientLogger()->trace(__VA_ARGS__)
#define KG_CRITICAL(...) ::Kargono::Log::GetClientLogger()->critical(__VA_ARGS__)

