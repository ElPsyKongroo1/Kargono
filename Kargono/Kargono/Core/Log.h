#pragma once
#include "Kargono/Core/Base.h"
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"
#pragma warning(pop)


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


template<typename OStream, glm::length_t L, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::vec<L, T, Q>& vector)
{
	return os << glm::to_string(vector);
}

template<typename OStream, glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::mat<C, R, T, Q>& matrix)
{
	return os << glm::to_string(matrix);
}

template<typename OStream, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, glm::qua<T, Q> quaternion)
{
	return os << glm::to_string(quaternion);
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

