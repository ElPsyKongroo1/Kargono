#include "kgpch.h"

#include "Kargono/Utility/Time.h"
#include "Kargono/Utility/OSCommands.h"
#include "Kargono/Utility/FileDialogs.h"
#include "FileSystemModule/FileSystem.h"
#include "Kargono/Core/Engine.h"


#if defined(KG_PLATFORM_LINUX) 

#include "API/Platform/LinuxBackendAPI.h"
#include "API/Platform/GlfwBackendAPI.h"

namespace Kargono::Utility
{
	static std::filesystem::path CallZenity(const std::string& command)
	{
		// Run command and receive pipe file with all logged messages (should just be the resultant file/folder)
		FILE* pipe = popen(command.c_str(), "r");

		// Ensure pipe file was generated
		if (!pipe)
		{
			KG_WARN("Could not access pipe connection between engine and zenity");
			return {};
		}

		// Get file/folder path from pipe
		char buffer[128];
		const char* text = fgets(buffer, sizeof(buffer), pipe);
		
		// Ensure text is returned by pipe
		if (!text)
		{
			KG_INFO("No file/folder path was returned from linux dialog");
			return {};
		}

		// Close the pipe and check for errors
		if (pclose(pipe) == -1) 
		{
			KG_WARN("Could not properly close pipe to file/folder dialog");
			return {};
		}

		// Remove trailing newline character and return path
		std::string outputText {text};
		outputText.pop_back();
		return std::filesystem::path(outputText);
	}

	std::filesystem::path FileDialogs::OpenFile(const char* filter, const char* initialDirectory)
	{
		// Establish command using zenity (wrapper around gtk on linux for file dialogs)
		std::string command = "zenity --file-selection --title=\"Open File\" ";

		// Optionally add an initial directory
		if (initialDirectory && strlen(initialDirectory) > 0)
		{
			command.append("--filename ");
			command.append(initialDirectory);
		}

		// Ensure no error messages are logged into the pipe
		command.append(" 2> /dev/null");

		return CallZenity(command);
	}
	std::filesystem::path FileDialogs::SaveFile(const char* filter, const char* initialDirectory)
	{
		// Establish command using zenity (wrapper around gtk on linux for file dialogs)
		std::string command = "zenity --file-selection --save --title=\"Save File\" ";

		// Optionally add an initial directory
		if (initialDirectory)
		{
			command.append("--filename ");
			command.append(initialDirectory);
		}

		// Ensure no error messages are logged into the pipe
		command.append(" 2> /dev/null");

		return CallZenity(command);
	}

	std::filesystem::path FileDialogs::ChooseDirectory(const std::filesystem::path& initialPath)
	{
		// Establish basic choose directory command using zenity (wrapper around gtk on linux for file dialogs)
		std::string command = "zenity --file-selection --directory --title=\"Choose Folder\" --filename ";
		command.append(initialPath.string());

		// Ensure no error messages are logged into the pipe
		command.append(" 2> /dev/null");

		return CallZenity(command);
	}
}

#endif
