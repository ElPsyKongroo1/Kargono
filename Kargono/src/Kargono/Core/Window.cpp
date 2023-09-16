#include "kgpch.h"

#include "Kargono/Core/Window.h"
#ifdef KG_PLATFORM_WINDOWS
#include "Platform/Windows/WindowsWindow.h"
#endif

namespace Kargono
{

	Scope<Window> Window::Create(const WindowProps& props)
	{
#ifdef KG_PLATFORM_WINDOWS
		return CreateScope<WindowsWindow>(props);
#else
		KG_CORE_ASSERT(false, "Unknown platform!");
		return nullptr;
#endif
	}

}
