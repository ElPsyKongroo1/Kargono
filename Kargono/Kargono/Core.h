#pragma once
#include "Kargono/kgpch.h"

#include <memory>

#ifdef KG_PLATFORM_WINDOWS
	#if KG_DYNAMIC_LINK
		#ifdef KG_BUILD_DLL
			#define KG_API __declspec(dllexport)	
		#else
			#define KG_API __declspec(dllimport)
		#endif
	#else
		#define KG_API
	#endif
#else
	#error Kargono currenatly only supports Windows
#endif

#ifdef KG_DEBUG
	#define KG_ENABLE_ASSERTS
#endif


#ifdef KG_ENABLE_ASSERTS
	#define KG_ASSERT(x, ...) {if (!(x)) {KG_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak();}}
	#define KG_CORE_ASSERT(x, ...) {if(!(x)) {KG_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak();}}
#else
	#define KG_ASSERT(x, ...) 
	#define KG_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

#define KG_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

namespace Kargono
{
	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T>
	using Ref = std::shared_ptr<T>;


}