#pragma once
#include "API/Profiler/OptickAPI.h"

namespace Kargono
{
#if KG_PROFILE == 1

	#define KG_PROFILE_FRAME(...) OPTICK_FRAME(__VA_ARGS__)

	#define KG_PROFILE_FUNCTION(...) OPTICK_EVENT(__VA_ARGS__) 

	#define KG_PROFILE_THREAD(...) OPTICK_THREAD(__VA_ARGS__)

	#define KG_PROFILE_SHUTDOWN() OPTICK_SHUTDOWN()
#else
	#define KG_PROFILE_FRAME(...)

	#define KG_PROFILE_FUNCTION(...)

	#define KG_PROFILE_THREAD(...)

	#define KG_PROFILE_SHUTDOWN()
#endif

}