#pragma once


#include <memory>
#include "Kargono/Core/PlatformDetection.h"

#ifdef KG_DEBUG
	#if defined(KG_PLATFORM_WINDOWS)
		#define KG_DEBUGBREAK() __debugbreak()
	#elif defined(KG_PLATFORM_LINUX)
		#include <signal.h>
		#define KG_DEBUGBREAK() raise(SIGTRAP)
	#else
		#error "Platform doesn't support debugbreak yet!"
	#endif
	#define KG_ENABLE_ASSERTS
#else
#define KG_DEBUGBREAK()
#endif

// TODO: Make this macro able to take in no arguments except exception
#ifdef KG_ENABLE_ASSERTS
	#define KG_ASSERT(x, ...) { if(!(x)) { KG_ERROR("Assertion Failed: {0}", __VA_ARGS__); KG_DEBUGBREAK(); } }
	#define KG_CORE_ASSERT(x, ...) { if(!(x)) { KG_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); KG_DEBUGBREAK(); } }
#else
	#define KG_ASSERT(x, ...)
	#define KG_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

#define KG_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

namespace Kargono {

	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

}

