#pragma once

#include <memory>

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

#define KG_EXPAND_MACRO(x) x
#define KG_STRINGIFY_MACRO(x) #x

#define BIT(x) (1 << x)

#define KG_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

namespace Kargono
{
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

