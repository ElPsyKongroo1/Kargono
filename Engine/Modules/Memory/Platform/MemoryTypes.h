#pragma once

namespace Kargono::Memory
{
#if defined(KG_PLATFORM_WINDOWS)
#include <windows.h>
using MemoryFileDesc = HANDLE;

#elif defined(KG_PLATFORM_LINUX)
using MemoryFileDesc = int;
#else
	error "Unknown platform, must be one of: windows or linux"
#endif
}
