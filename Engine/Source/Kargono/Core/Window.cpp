#include "kgpch.h"

#include "Kargono/Core/Window.h"
#ifdef KG_PLATFORM_WINDOWS
#include "API/Platform/WindowsWindow.h"
#endif

namespace Kargono
{

	Scope<Window> Window::Create(const WindowProps& props)
	{
#if defined(KG_PLATFORM_WINDOWS)
		return CreateScope<API::Platform::WindowsWindow>(props);
#elif defined(KG_PLATFORM_LINUX)
		return nullptr;
#else
	#error "Unknown platform!"	
#endif
	}

}
