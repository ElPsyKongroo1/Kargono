#pragma once

#include <filesystem>

namespace Kargono::Scripting
{
#if defined(KG_PLATFORM_WINDOWS)
#include <windows.h>
#endif

#if defined(KG_PLATFORM_WINDOWS)
	using SharedLibHandle = HINSTANCE*;
#elif defined(KG_PLATFORM_LINUX)
	using SharedLibHandle = void*;
#endif

#if defined(KG_PLATFORM_WINDOWS)
	using SharedLibFuncPtr = FARPROC;
#elif defined(KG_PLATFORM_LINUX)
	using SharedLibFuncPtr = void*;
#endif
}