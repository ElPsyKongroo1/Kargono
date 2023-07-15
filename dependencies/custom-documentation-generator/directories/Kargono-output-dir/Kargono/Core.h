/// @brief Preprocessor directive to include the file only once
#pragma once
/// @brief Include the Kargono/kgpch.h file
#include "Kargono/kgpch.h"

/// @brief Conditional compilation statement based on KG_PLATFORM_WINDOWS
/// @brief Conditional compilation statement for dynamic linking
/// @brief  Defines a macro based on KG_BUILD_DLL definition
/// @brief Conditional compile statement based on KG_DEBUG definition
#ifdef KG_PLATFORM_WINDOWS
	#if KG_DYNAMIC_LINK
		#ifdef KG_BUILD_DLL
/// @brief Define a macro based on KG_API definition
/// @brief Define an empty macro
/// @brief Define KG_ENABLE_ASSERTS macro
/// @brief Define KG_ASSERT macro for runtime assertions
/// @brief Define KG_CORE_ASSERT macro for core assertions
/// @brief Define KG_ASSERT macro as empty macro
/// @brief Define KG_CORE_ASSERT macro as empty macro
/// @brief Define a macro to set a specific bit in an integer
/// @brief Define KG_BIND_EVENT_FN macro to bind an event function
			#define KG_API __declspec(dllexport)	
/// @brief The KG_API macro is not defined
/// @brief Platform is not Windows
/// @brief KG_ENABLE_ASSERTS is not defined
		#else
			#define KG_API __declspec(dllimport)
/// @brief Close the inner #ifdef directive
/// @brief Close the outer #ifdef directive
/// @brief Close the #ifdef directive
/// @brief Close the else #ifdef directive
		#endif
	#else
		#define KG_API
	#endif
#else
/// @brief Exit compilation with an error message
	#error Kargono currently only supports Windows
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