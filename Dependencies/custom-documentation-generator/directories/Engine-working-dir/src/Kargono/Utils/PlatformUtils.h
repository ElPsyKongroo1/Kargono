#pragma once
#include <string>
#include <filesystem>


namespace Kargono
{

	class FileExplorer
	{
	public:
		static void OpenFileExplorer( const std::filesystem::path& path);
		static void OpenScriptProject( const std::filesystem::path& path);
	};

	class FileDialogs
	{
	public:
		// These return empty string if cancelled
		static std::string OpenFile(const char* filter);
		static std::string SaveFile(const char* filter);
	};

	class Time
	{
	public:
		static float GetTime();
	};
}
