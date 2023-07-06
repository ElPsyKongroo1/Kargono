/// @brief Preprocessor directive to include this file only once during compilation
#pragma once


/// @brief Includes the header file for std::shared_ptr
#include <memory>
/// @brief Includes the header file for std::utility
#include <utility>
/// @brief Includes the header file for std::algorithm
#include <algorithm>
/// @brief Includes the header file for std::functional
#include <functional>

/// @brief Includes the header file for std::string
#include <string>
/// @brief Includes the header file for std::stringstream
#include <sstream>
/// @brief Includes the header file for std::vector
#include <vector>
/// @brief Includes the header file for std::unordered_map
#include <unordered_map>
/// @brief Includes the header file for std::unordered_set
#include <unordered_set>

/// @brief Includes the header file for std::cmath
#include <cmath>
#include <vector>
/// @brief Includes the header file for std::cassert
#include <cassert>
/// @brief Includes the header file for std::array
#include <array>
/// @brief Includes the header file for std::limits
#include <limits>
/// @brief Includes the header file for std::cstdint
#include <cstdint>
/// @brief Includes the header file for std::fstream
#include <fstream>

// P_ISSUE: Used to disable .dll export warning for shared_ptr's
/// @brief Disables a specific warning that occurs during compilation
#pragma warning(disable: 4251)


/// @brief Preprocessor directive to check for the Windows platform
#ifdef KG_PLATFORM_WINDOWS
/// @brief Includes the header file for Windows API
	#include <Windows.h>
/// @brief End of the preprocessor directive block for the Windows platform
#endif


