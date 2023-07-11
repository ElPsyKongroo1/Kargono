#pragma once
#include "Kargono/kgpch.h"

#ifdef KG_PLATFORM_WINDOWS
	#ifdef KG_BUILD_DLL
		#define KG_API __declspec(dllexport)	
	#else
		#define KG_API __declspec(dllimport)
	#endif
#else
	#error Kargono currently only supports Windows
#endif

#ifdef KG_ENABLE_ASSERTS
	#define KG_ASSERT(x, ...) {if (!(x)) {KG_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak();}}
	#define KG_CORE_ASSERT(x, ...) {if(!(x)) {KG_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak();}}
#else
	#define KG_ASSERT(x, ...) 
	#define KG_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)