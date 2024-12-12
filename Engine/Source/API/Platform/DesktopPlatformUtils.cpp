#include "kgpch.h"

#include "Kargono/Utility/Time.h"
#include "Kargono/Utility/OSCommands.h"
#include "Kargono/Utility/FileDialogs.h"
#include "Kargono/Utility/FileSystem.h"
#include "Kargono/Core/Engine.h"

#if defined(KG_PLATFORM_WINDOWS) || defined(KG_PLATFORM_LINUX) 

#include "API/Platform/GlfwAPI.h"

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

#if !defined(KG_EXPORT_SERVER) && !defined(KG_EXPORT_RUNTIME)
	void OSCommands::OpenFileExplorer(const std::filesystem::path& path)
	{
		KG_ASSERT(std::filesystem::is_directory(path), "Invalid path provided, needs to be a directory!");
		// TODO: Add More Input Validation for system call.
		std::string outputString = "explorer " + path.string();
		system(outputString.c_str());
	}

	void OSCommands::OpenTerminal(const std::filesystem::path& path)
	{
		KG_ASSERT(std::filesystem::is_directory(path), "Invalid path provided, needs to be a directory!");

		std::string command;

		// Open Command Prompt at the specified directory
		command = "start cmd /K \"cd /d " + path.string() + "\"";

		// Execute the command to open the terminal
		system(command.c_str());
	}

	void OSCommands::OpenScriptProject(const std::filesystem::path& path)
	{
		// TODO: Add More Input Validation for system call.
		if (!Utility::FileSystem::PathExists(path))
		{
			KG_ERROR("Invalid path provided to OpenScriptProject");
			return;
		}
		std::string outputString = "start " + path.string();
		system(outputString.c_str());
	}

	void OSCommands::OpenProfiler()
	{
		std::string outputString = "start " + std::string("../Dependencies/optick/Optick.exe");
		system(outputString.c_str());
	}

	void OSCommands::DownloadGitProject(const std::filesystem::path& downloadPath, const std::string& projectURI)
	{
		std::string outputString = "git clone " + projectURI + " " + downloadPath.string();
		system(outputString.c_str());
	}

	void OSCommands::OpenWebURL(const std::string& webURL)
	{
		std::string op = std::string("start ").append(webURL);
		system(op.c_str());
	}

#endif
}

#endif