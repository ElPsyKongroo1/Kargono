#include "kgpch.h"

#include "Kargono/Utility/Time.h"
#include "Kargono/Utility/OSCommands.h"
#include "Kargono/Utility/FileDialogs.h"
#include "Kargono/Utility/FileSystem.h"
#include "Kargono/Core/Engine.h"


#if defined(KG_PLATFORM_LINUX) 

#include "API/Platform/LinuxBackendAPI.h"

namespace Kargono::Utility
{
    // TODO: Replace function stubs with actual linux implementations
	std::filesystem::path FileDialogs::OpenFile(const char* filter, const char* initialDirectory)
	{
		return {};
	}
	std::filesystem::path FileDialogs::SaveFile(const char* filter, const char* initialDirectory)
	{
		return {};
	}

	std::filesystem::path FileDialogs::ChooseDirectory(const std::filesystem::path& initialPath)
	{
		return {};
	}
}

#endif
