/// @brief Guard for preventing multiple inclusion of header files
#pragma once


/// @brief Provides classes and functions for managing dynamic memory
#include <memory>
/// @brief Provides utilities for working with pairs and tuples
#include <utility>
/// @brief Provides a collection of algorithms for working with sequences
#include <algorithm>
/// @brief Provides a collection of function objects
#include <functional>

/// @brief Provides classes and functions for working with strings
#include <string>
/// @brief Provides classes and functions for working with string streams
#include <sstream>
/// @brief Provides a dynamic array-like container
/// @brief Provides mathematical functions and constants
#include <vector>
/// @brief Provides a hash-based map container
#include <unordered_map>
/// @brief Provides a hash-based set container
#include <unordered_set>

/// @brief Provides mathematical functions and constants
#include <cmath>
#include <vector>
/// @brief Provides macro for assertion
#include <cassert>
/// @brief Provides a fixed-size array-like container
#include <array>
/// @brief Provides numeric limits
#include <limits>
/// @brief Provides fixed-width integer types
#include <cstdint>
/// @brief Provides classes and functions for working with files
#include <fstream>

// P_ISSUE: Used to disable .dll export warning for shared_ptr's
/// @brief Disables warning related to DLL export for shared_ptr's
#pragma warning(disable: 4251)
/// @brief Disables warning related to deprecated functions
#pragma warning(disable: 4996)

/// @brief Disables warning related to missing prototypes
#pragma warning(disable: 4013)


/// @brief Platform-specific code for Windows operating system
#ifdef KG_PLATFORM_WINDOWS
/// @brief Provides WinAPI functions and types
	#include <Windows.h>
#endif


