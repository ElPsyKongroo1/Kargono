#pragma once
#include <string>
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
#if KG_EXPORT == 0
		// This function opens a file explorer window at the provided absolute
		//		path. This is operating system dependent and only works on windows.
		static void OpenFileExplorer( const std::filesystem::path& path);
		// This function opens the scripting engine that the provided path describes.
		//		This scripting engine is a .csproj file that opens the Visual Studio
		//		project.
		static void OpenScriptProject( const std::filesystem::path& path);
#endif
	};

	//============================================================
	// File Dialog Class
	//============================================================
	// This class provides static functions that open file dialogs in the
	//		current operating system to select files that can be used
	//		in the engine.
	class FileDialogs
	{
	public:
		// This function opens a file dialog box that requests the user
		//		to select a file whose extension matches the filter provided
		//		in the parameter. The selected file can be later handled
		//		in the calling code.
		static std::filesystem::path OpenFile(const char* filter);
		// This function opens a file dialog box that requests the user
		//		to select a directory and filename to save a file with.
		//		The selected path is returned to the calling code to
		//		be handled.
		static std::filesystem::path SaveFile(const char* filter);
	};

	//============================================================
	// Time Class
	//============================================================
	// This class provides static functions dealing with time such as
	//		getting the current time values in seconds since the application
	//		started.
	class Time
	{
	public:
		// This function returns the time since that application has started
		//		in seconds (Technically, the time since GLFW was initialized).
		static float GetTime();
	};
}
