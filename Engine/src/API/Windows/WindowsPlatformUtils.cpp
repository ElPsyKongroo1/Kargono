#include "kgpch.h"

#include "Kargono/Utility/Time.h"
#include "Kargono/Utility/OSCommands.h"
#include "Kargono/Utility/FileDialogs.h"
#include "Kargono/Utility/FileSystem.h"
#include "Kargono/Core/Core.h"

#include <windows.h>
#include <shellapi.h>
#include <commdlg.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>



namespace Kargono::Utility
{

	float Time::GetTime()
	{
		return (float)glfwGetTime();
	}

	void Time::GetTimeFromMilliseconds(uint64_t initialMilliseconds, uint64_t& hours, uint64_t& minutes, uint64_t& seconds, uint64_t& milliseconds)
	{
		// Conversion Values
		constexpr uint64_t hoursToMilliseconds{ 3'600'000 };
		constexpr uint64_t minutesToMilliseconds{ 60'000 };
		constexpr uint64_t secondsToMilliseconds{ 1000 };

		// Convert Values
		milliseconds = initialMilliseconds;
		hours = milliseconds / hoursToMilliseconds;
		milliseconds %= hoursToMilliseconds;
		minutes = milliseconds / minutesToMilliseconds;
		milliseconds %= minutesToMilliseconds;
		seconds = milliseconds / secondsToMilliseconds;
		milliseconds %= secondsToMilliseconds;
	}

	std::string Time::GetStringFromMilliseconds(uint64_t initialMilliseconds)
	{
		// Conversion Values
		constexpr uint64_t hoursToMilliseconds{ 3'600'000 };
		constexpr uint64_t minutesToMilliseconds{ 60'000 };
		constexpr uint64_t secondsToMilliseconds{ 1000 };

		// Convert Values
		uint64_t milliseconds = initialMilliseconds;
		uint64_t hours = milliseconds / hoursToMilliseconds;
		milliseconds %= hoursToMilliseconds;
		uint64_t minutes = milliseconds / minutesToMilliseconds;
		milliseconds %= minutesToMilliseconds;
		uint64_t seconds = milliseconds / secondsToMilliseconds;
		milliseconds %= secondsToMilliseconds;
		std::stringstream timeOutput{};
		if (hours > 0)
		{
			if (hours == 1) { timeOutput << "1 Hour"; }
			else { timeOutput << std::to_string(hours) << " Hours"; }
			if (milliseconds != 0 || seconds != 0 || minutes != 0) { timeOutput << ", "; }
		}

		if (minutes > 0)
		{
			if (minutes == 1) { timeOutput << "1 Minute"; }
			else { timeOutput << std::to_string(minutes) << " Minutes"; }
			if (milliseconds != 0 || seconds != 0) { timeOutput << ", "; }
		}

		if (seconds > 0)
		{
			if (seconds == 1) { timeOutput << "1 Second"; }
			else { timeOutput << std::to_string(seconds) << " Seconds"; }
			if (milliseconds != 0) { timeOutput << ", "; }

		}

		if (milliseconds > 0 || hours + minutes + seconds == 0)
		{
			if (milliseconds == 1) { timeOutput << "1 Millisecond"; }
			else { timeOutput << std::to_string(milliseconds) << " Milliseconds"; }
		}

		return timeOutput.str();
	}

	std::string Time::GetStringFromSeconds(uint64_t initialSeconds)
	{
		// Conversion Values
		constexpr uint64_t hoursToSeconds{ 3600 };
		constexpr uint64_t minutesToSeconds{ 60 };

		// Convert Values
		uint64_t seconds = initialSeconds;
		uint64_t hours = seconds / hoursToSeconds;
		seconds %= hoursToSeconds;
		uint64_t minutes = seconds / minutesToSeconds;
		seconds %= minutesToSeconds;

		std::stringstream timeOutput{};
		if (hours > 0)
		{
			if (hours == 1) { timeOutput << "1 Hour"; }
			else { timeOutput << std::to_string(hours) << " Hours"; }
			if (seconds != 0 || minutes != 0) { timeOutput << ", "; }
		}

		if (minutes > 0)
		{
			if (minutes == 1) { timeOutput << "1 Minute"; }
			else { timeOutput << std::to_string(minutes) << " Minutes"; }
			if (seconds != 0) { timeOutput << ", "; }
		}

		if (seconds > 0 || hours + minutes == 0)
		{
			if (seconds == 1) { timeOutput << "1 Second"; }
			else { timeOutput << std::to_string(seconds) << " Seconds"; }

		}

		return timeOutput.str();
	}





#if KG_EXPORT == 0
	void OSCommands::OpenFileExplorer(const std::filesystem::path& path)
	{
		KG_ASSERT(std::filesystem::is_directory(path), "Invalid path provided, needs to be a directory!");
		// TODO: Add More Input Validation for system call.
		std::string outputString = "explorer " + path.string();
		system(outputString.c_str());
	}

	void OSCommands::OpenScriptProject(const std::filesystem::path& path)
	{
		// TODO: Add More Input Validation for system call.
		if (!std::filesystem::exists(path))
		{
			KG_ERROR("Invalid path provided to OpenScriptProject");
			return;
		}
		std::string outputString = "start " + path.string();
		system(outputString.c_str());
	}

	void OSCommands::OpenProfiler()
	{
		std::string outputString = "start " + std::string("../Engine/dependencies/optick/Optick.exe");
		system(outputString.c_str());
	}



#endif

	std::filesystem::path FileDialogs::OpenFile(const char* filter, const char* initialDirectory)
	{
		
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		CHAR currentDir[256] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Core::GetCurrentApp().GetWindow().GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		if (initialDirectory == "")
		{
			if (GetCurrentDirectoryA(256, currentDir))
			{
				ofn.lpstrInitialDir = currentDir;
			}
		}
		else
		{
			ofn.lpstrInitialDir = initialDirectory;
		}
		if (filter != "")
		{
			ofn.lpstrFilter = filter;
		}
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetOpenFileNameA(&ofn) == TRUE)
			return ofn.lpstrFile;

		return std::string();

	}

		CHAR szFile[260] = { 0 };
	std::filesystem::path FileDialogs::SaveFile(const char* filter, const char* initialDirectory)
	{
		OPENFILENAMEA ofn;
		CHAR currentDir[256] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Core::GetCurrentApp().GetWindow().GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);

		if (initialDirectory == "")
		{
			if (GetCurrentDirectoryA(256, currentDir))
			{
				ofn.lpstrInitialDir = currentDir;
			}
		}
		else
		{
			ofn.lpstrInitialDir = initialDirectory;
		}

		if (filter != "")
		{
			ofn.lpstrFilter = filter;
		}
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

		// Sets the default extension by extracting it from the filter
		ofn.lpstrDefExt = strchr(filter, '\0') + 1;

		if (GetSaveFileNameA(&ofn) == TRUE)
			return ofn.lpstrFile;

		return std::string();
	}

}
