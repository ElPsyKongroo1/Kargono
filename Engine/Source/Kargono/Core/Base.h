#pragma once

#include <memory>
#include <stdexcept>

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

#define KG_BIND_CLASS_FN(fn) [this](auto&&... args) -> decltype(auto) \
{ \
	return this->fn(std::forward<decltype(args)>(args)...); \
}

#include "Kargono/Core/Types.h"
#include "Kargono/Core/Log.h"
#include "Kargono/Core/Assert.h"
#include "Kargono/Core/Profiler.h"
#include "Kargono/Core/FixedString.h"

#include <filesystem>

