#pragma once

#ifdef KG_DEBUG
#define KG_PROFILE 1 // Switch this to turn profiling on/off
#else
#define KG_PROFILE 0
#endif



#if KG_PROFILE == 1
#include "optick.h"
#endif

// Here is the optick API reference: https://github.com/bombomby/optick/wiki/Optick-API

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
