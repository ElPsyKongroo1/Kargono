/// @brief Prevents multiple inclusion of this file in the same translation unit.
#pragma once


/// @brief Include the header for smart pointers.
#include <memory>
/// @brief Include the header for utility functions.
#include <utility>
/// @brief Include the header for algorithms.
#include <algorithm>
/// @brief Include the header for function objects.
#include <functional>

/// @brief Include the header for strings.
#include <string>
/// @brief Include the header for string streams.
#include <sstream>
/// @brief Include the header for vectors.
#include <vector>
/// @brief Include the header for unordered maps.
#include <unordered_map>
/// @brief Include the header for unordered sets.
#include <unordered_set>

/// @brief Include the header for mathematical functions.
#include <cmath>
#include <vector>
/// @brief Include the header for assertions.
#include <cassert>
/// @brief Include the header for arrays.
#include <array>
/// @brief Include the header for numeric limits.
#include <limits>
/// @brief Include the header for fixed-size integer types.
#include <cstdint>
/// @brief Include the header for file streams.
#include <fstream>

// P_ISSUE: Used to disable .dll export warning for shared_ptr's
/// @brief Disable the warning for shared_ptr's usage within .dll export.
#pragma warning(disable: 4251)


/// @brief Preprocessor directive for Windows platform.
#ifdef KG_PLATFORM_WINDOWS
/// @brief Include the header for Windows API.
	#include <Windows.h>
#endif


