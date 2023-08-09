#pragma once

#include "Kargono/Core/PlatformDetection.h"

#ifdef KG_PLATFORM_WINDOWS
	#ifndef NOMINMAX
		// See github.com/skypjack/entt/wiki/Frequently-Asked-Questions#warning-c4003-the-min-the-max-and-the-macro
		#define NOMINMAX
	#endif
#endif


#include <memory>
#include <utility>
#include <algorithm>
#include <functional>

#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include <cmath>
#include <cassert>
#include <array>
#include <limits>
#include <cstdint>
#include <fstream>

#include "Kargono/Core/Base.h"
#include "Kargono/Core/Log.h"
#include "Kargono/Debug/Instrumentor.h"

// P_ISSUE: Used to disable .dll export warning for shared_ptr's
//#pragma warning(disable: 4251)
//#pragma warning(disable: 4996)
//
//#pragma warning(disable: 4013)


#ifdef KG_PLATFORM_WINDOWS
	#include <Windows.h>
#endif


