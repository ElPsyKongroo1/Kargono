#pragma once
#include "Core.h"
#include <spdlog/spdlog.h>
#include "spdlog/fmt/ostr.h"


namespace Kargono
{
	class KG_API Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
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
#define KG_ERROR(...) ::Kargono::Log::GetClientLogger()->error(__VA_ARGS__)
#define KG_WARN(...) ::Kargono::Log::GetClientLogger()->warn(__VA_ARGS__)
#define KG_INFO(...) ::Kargono::Log::GetClientLogger()->info(__VA_ARGS__)
#define KG_TRACE(...) ::Kargono::Log::GetClientLogger()->trace(__VA_ARGS__)
#define KG_FATAL(...) ::Kargono::Log::GetClientLogger()->fatal(__VA_ARGS__)

