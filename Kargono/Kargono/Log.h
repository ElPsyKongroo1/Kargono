#pragma once
#include "Core.h"
#include <spdlog/spdlog.h>
#include <memory>

namespace Karg{
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
#define KG_CORE_ERROR(...)	::Karg::Log::GetCoreLogger()->error(__VA_ARGS__)
#define KG_CORE_WARN(...)	::Karg::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define KG_CORE_INFO(...)	::Karg::Log::GetCoreLogger()->info(__VA_ARGS__)
#define KG_CORE_TRACE(...)	::Karg::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define KG_CORE_FATAL(...)	::Karg::Log::GetCoreLogger()->fatal(__VA_ARGS__)

// Client Log Macros
#define KG_ERROR(...) ::Karg::Log::GetClientLogger()->error(__VA_ARGS__)
#define KG_WARN(...) ::Karg::Log::GetClientLogger()->warn(__VA_ARGS__)
#define KG_INFO(...) ::Karg::Log::GetClientLogger()->info(__VA_ARGS__)
#define KG_TRACE(...) ::Karg::Log::GetClientLogger()->trace(__VA_ARGS__)
#define KG_FATAL(...) ::Karg::Log::GetClientLogger()->fatal(__VA_ARGS__)

