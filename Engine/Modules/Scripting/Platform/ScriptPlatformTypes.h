#pragma once

namespace Kargono::Scripting
{
#if defined(KG_PLATFORM_WINDOWS)
#include <windows.h>
	using SharedLibHandle = HINSTANCE*;
#elif defined(KG_PLATFORM_LINUX)
	using SharedLibHandle = void*;
#endif

	struct SharedLib
	{
		SharedLibHandle m_Handle{ nullptr };
	};

}