#pragma once

#include <memory>
#include <stdexcept>
#include <string>

namespace Kargono
{
	//==============================
	// Testing Types/Data
	//==============================
	inline bool s_TestingActive = false;
}

// Fail-safe macro code to ensure correct defines are provided
#if !defined(KG_PLATFORM_WINDOWS) && !defined(KG_PLATFORM_LINUX) && !defined(KG_PLATFORM_MAC)

	#if defined(_WIN32) || defined(_WIN64)
		#define KG_PLATFORM_WINDOWS
    #elif defined(__linux__)
		#define KG_PLATFORM_LINUX
    #elif defined(__APPLE__) && defined(__MACH__)
		#define KG_PLATFORM_MAC
    #else
    #endif

#endif

#if defined(KG_PLATFORM_WINDOWS)
#define KG_DEBUGBREAK() \
	if (!Kargono::s_TestingActive) \
	{ \
		__debugbreak(); \
	}
#define KG_DEBUGBREAK_MSG(msg) \
	if (!Kargono::s_TestingActive) \
	{ \
		__debugbreak(); \
	}
#elif defined(KG_PLATFORM_LINUX)
#include <signal.h>
#define KG_DEBUGBREAK() \
  if (!Kargono::s_TestingActive) \
  { \
	raise(SIGTRAP); \
  }
#define KG_DEBUGBREAK_MSG(msg) \
  if (!Kargono::s_TestingActive) \
  { \
	raise(SIGTRAP); \
  }
#else
#error "Platform doesn't support debugbreak yet!"
#endif
#define KG_ENABLE_VERIFY

#ifdef KG_DEBUG
	#define KG_ENABLE_ASSERTS
#endif

#define KG_EXPAND_MACRO(x) x
#define KG_STRINGIFY_MACRO(x) #x

#define BIT(x) (1 << x)

#define KG_BIND_CLASS_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }



namespace Kargono
{
	//==============================
	// Engine Types
	//==============================
	// Definition of Scope, which is a wrapper for a Unique Pointer
	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	constexpr Scope<T> CreateScope(T* pointer)
	{
		return std::unique_ptr<T>(pointer);
	}
	// Definition of Ref, which is a wrapper for a Shared Pointer
	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	constexpr Ref<T> CreateRef(T* pointer)
	{
		return std::shared_ptr<T>(pointer);
	}

	// Definition of Weak, which is a wrapper for a Weak Pointer
	template<typename T>
	using Weak = std::weak_ptr<T>;

	template<typename T>
	constexpr Weak<T> CreateWeak(Ref<T> reference)
	{
		std::weak_ptr<T> weakReference = reference;
		return weakReference;
	}

	template<typename T>
	constexpr Weak<T> CreateWeak(Scope<T> scope)
	{
		std::weak_ptr<T> weakReference = scope;
		return weakReference;
	}

}

#include "Kargono/Core/Log.h"
#include "Kargono/Core/Assert.h"
#include "Kargono/Core/Profiler.h"
#include "Kargono/Core/FixedString.h"

#include <filesystem>

