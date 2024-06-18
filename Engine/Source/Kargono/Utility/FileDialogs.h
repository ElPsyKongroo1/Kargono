#pragma once

namespace Kargono::Utility
{

	//============================================================
	// File Dialog Class
	//============================================================
	// This class provides static functions that open file dialogs in the
	//		current operating system to select files that can be used
	//		in the engine.
	class FileDialogs
	{
	public:
		//==============================
		// Create File Dialog Functions
		//==============================
		// This function opens a file dialog box that requests the user
		//		to select a file whose extension matches the filter provided
		//		in the parameter. The selected file can be later handled
		//		in the calling code.
		static std::filesystem::path OpenFile(const char* filter = "", const char* initialDirectory = "");
		// This function opens a file dialog box that requests the user
		//		to select a directory and filename to save a file with.
		//		The selected path is returned to the calling code to
		//		be handled.
		static std::filesystem::path SaveFile(const char* filter = "", const char* initialDirectory = "");
	};
}
