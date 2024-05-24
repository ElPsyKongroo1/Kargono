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
