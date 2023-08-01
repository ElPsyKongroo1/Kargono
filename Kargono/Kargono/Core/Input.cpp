#include "Kargono/kgpch.h"
#include "Kargono/Core/Input.h"

#ifdef KG_PLATFORM_WINDOWS
#include "Platform/Windows/WindowsInput.h"
#endif

namespace Kargono {

	Scope<Input> Input::s_Instance = Input::Create();

	Scope<Input> Input::Create()
	{
	#ifdef KG_PLATFORM_WINDOWS
			return CreateScope<WindowsInput>();
	#else
			KG_CORE_ASSERT(false, "Unknown platform!");
			return nullptr;
	#endif
	}
}