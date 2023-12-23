#include "kgpch.h"

#include "Kargono/Core/Window.h"
#ifdef KG_PLATFORM_WINDOWS
#include "API/Windows/WindowsWindow.h"
#endif

namespace Kargono
{

	Scope<Window> Window::Create(const WindowProps& props)
	{
#ifdef KG_PLATFORM_WINDOWS
		return CreateScope<API::Windows::WindowsWindow>(props);
#else
		KG_CORE_ASSERT(false, "Unknown platform!");
		return nullptr;
#endif
	}

}
