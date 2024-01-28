#pragma once

#ifdef KG_PLATFORM_WINDOWS
	#ifndef NOMINMAX
		// See github.com/skypjack/entt/wiki/Frequently-Asked-Questions#warning-c4003-the-min-the-max-and-the-macro
		#define NOMINMAX
	#endif
#endif

#include <iostream>
#include <filesystem>
#include <memory>
#include <chrono>
#include <utility>
#include <algorithm>
#include <functional>
#include <exception>
#include <thread>
#include <mutex>
#include <atomic>

#include <string>
#include <string_view>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <map>
#include <queue>
#include <unordered_set>
#include <tuple>
#include <array>
#include <set>
#include <deque>

#include <cmath>
#include <cassert>
#include <limits>
#include <cstdint>
#include <fstream>
#include <random>

#include "Kargono/Core/Base.h"
#include "Kargono/Core/Log.h"
#include "Kargono/Core/UUID.h"







