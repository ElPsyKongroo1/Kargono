#include "kgpch.h"

#include "Modules/Scripting/Platform/SharedLibrary.h"

#if defined(KG_PLATFORM_WINDOWS)
#include "API/Platform/WindowsBackendAPI.h"
#elif defined(KG_PLATFORM_LINUX)
#include "API/Platform/LinuxBackendAPI.h"
#else
#error "Platform not supported"
#endif

namespace Kargono::Scripting
{
	bool SharedLib::LoadSharedLib(const char* libLocation)
	{
#if defined(KG_PLATFORM_WINDOWS)
		m_Handle = new HINSTANCE();
		*m_Handle = LoadLibraryA(libLocation);
#elif defined(KG_PLATFORM_LINUX)
		m_Handle = dlopen(libLocation, RTLD_LAZY);
#endif
		if (!IsActive())
		{
			return false;
		}

		return true;
	}
	bool SharedLib::CloseSharedLib()
	{
		if (!IsActive())
		{
			return false;
		}

#if defined(KG_PLATFORM_WINDOWS)
		FreeLibrary(*m_Handle);
		delete m_Handle;
#elif defined(KG_PLATFORM_LINUX)
		dlclose(m_Handle);
#endif
		m_Handle = nullptr;

		return true;
	}
	SharedLibFuncPtr SharedLib::GetFuncPtrRaw(const char* funcIdentifier)
	{
#if defined(KG_PLATFORM_WINDOWS)
		return GetProcAddress(*m_Handle, funcIdentifier);
#elif defined(KG_PLATFORM_LINUX)
		return dlsym(m_ScriptLibrary.m_Handle, funcIdentifier);
#endif
	}
	bool SharedLib::IsActive()
	{
		if (!(bool)m_Handle)
		{
			return false;
		}

#if defined(KG_PLATFORM_WINDOWS)
		if (*m_Handle == NULL)
		{
			return false;
		}
#endif
		return true;
	}
}