#pragma once
#include <filesystem>

//============================================================
// Utility Namespace
//============================================================
// This namespace holds many different utility functions for different
//		classes. This namespace is general purpose and generally holds
//		conversion functions that allow other functionality to work.
//		This namespace also holds certain operating system dependent
//		commands such as opening a file explorer or dialog.
namespace Kargono::Utility
{
	//============================================================
	// Operating System Commands Class
	//============================================================
	// This class provides commands that run different operating system dependent
	//		commands such as opening a file explorer or opening a scripting project.
	class OSCommands
	{
	public:

#if !defined(KG_EXPORT_SERVER) && !defined(KG_EXPORT_RUNTIME)
		// This function opens a file explorer window at the provided absolute
		//		path. This is operating system dependent and only works on windows.
		static void OpenFileExplorer(const std::filesystem::path& path);
		// This function opens the scripting engine that the provided path describes.
		//		This scripting engine is a .csproj file that opens the Visual Studio
		//		project.
		static void OpenScriptProject(const std::filesystem::path& path);
		// This function opens the profiler associated with this engine (optick).
		static void OpenProfiler();
#endif
	};

}
