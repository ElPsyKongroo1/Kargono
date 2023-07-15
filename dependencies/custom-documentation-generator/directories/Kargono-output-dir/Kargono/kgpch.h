/// @brief Ensures that this header file is only included once in the compilation process
#pragma once


/// @brief Includes the memory library, which provides features for managing dynamic memory
#include <memory>
/// @brief Includes the utility library, which provides various utility functions and classes
#include <utility>
/// @brief Includes the algorithm library, which provides a collection of functions for working with ranges of elements
#include <algorithm>
/// @brief Includes the functional library, which provides function objects and function adaptors
#include <functional>

/// @brief Includes the string library, which provides classes and functions for working with strings
#include <string>
/// @brief Includes the sstream library, which provides classes and functions for string streams
#include <sstream>
/// @brief Includes the vector library, which provides dynamic array-like functionality
#include <vector>
/// @brief Includes the unordered_map library, which provides an associative container that stores elements formed by the combination of a key and a mapped value
#include <unordered_map>
/// @brief Includes the unordered_set library, which provides an associative container that contains a set of unique objects
#include <unordered_set>

/// @brief Includes the cmath library, which provides mathematical operations and functions
#include <cmath>
#include <vector>
/// @brief Includes the cassert library, which provides macro functions for debugging and testing assertions
#include <cassert>
/// @brief Includes the array library, which provides a container that encapsulates arrays of a fixed size
#include <array>
/// @brief Includes the limits library, which provides functions for working with implementation-dependent limits of fundamental integral types
#include <limits>
/// @brief Includes the cstdint library, which provides types with fixed sizes and defined behavior across different platforms
#include <cstdint>
/// @brief Includes the fstream library, which provides classes and functions for file input and output
#include <fstream>

// P_ISSUE: Used to disable .dll export warning for shared_ptr's
//#pragma warning(disable: 4251)
//#pragma warning(disable: 4996)
//
//#pragma warning(disable: 4013)


/// @brief Checks if the Windows platform is being used
#ifdef KG_PLATFORM_WINDOWS
/// @brief Includes the Windows.h header, which contains declarations for all of the functions in the Windows API
	#include <Windows.h>
#endif


