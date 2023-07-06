/// @brief Ensures that this file is included only once
#pragma once
/// @brief Include the precompiled header file
#include "Kargono/kgpch.h"

/// @brief Conditionally compiled for the Windows platform
#ifdef KG_PLATFORM_WINDOWS
/// @brief Conditionally defined for building the Kargono engine as a DLL
	#ifdef KG_BUILD_DLL
/// @brief Defines the KG_API as __declspec(dllexport) for exporting the Kargono engine as a DLL
		#define KG_API __declspec(dllexport)	
/// @brief Conditionally compiled when not building the Kargono engine as a DLL
/// @brief Compiled when not running on the Windows platform
/// @brief Compiles when asserts are not enabled
	#else
/// @brief Defines the KG_API as __declspec(dllimport) for importing the Kargono engine as a DLL
		#define KG_API __declspec(dllimport)
/// @brief Ends the conditional compilation for building the Kargono engine as a DLL
/// @brief Ends the conditional compilation for the Windows platform
/// @brief Ends the conditional compilation for assert checking
	#endif
#else
/// @brief Stops compilation with an error message when trying to run on a platform other than Windows
	#error Kargono currently only supports Windows
#endif

/// @brief Checks whether asserts are enabled
#ifdef KG_ENABLE_ASSERTS
/// @brief Macro for runtime assertion checking with error message and breakpoint if the condition is not true
/// @brief Null operation for assertion check when asserts are disabled
	#define KG_ASSERT(x, ...) {if (!(x)) {KG_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak();}}
/// @brief Macro for runtime core assertion checking with error message and breakpoint if the condition is not true
/// @brief Null operation for core assertion check when asserts are disabled
	#define KG_CORE_ASSERT(x, ...) {if(!(x)) {KG_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak();}}
#else
	#define KG_ASSERT(x, ...) 
	#define KG_CORE_ASSERT(x, ...)
#endif

/// @brief Macro for creating bit masks
#define BIT(x) (1 << x)