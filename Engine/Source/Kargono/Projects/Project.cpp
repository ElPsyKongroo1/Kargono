#include "kgpch.h"

#include "Kargono/Projects/Project.h"

#include "Kargono/Core/Engine.h"
#include "Kargono/Assets/AssetManager.h"

#ifdef KG_PLATFORM_WINDOWS
#include "API/Platform/WindowsBackendAPI.h"
#endif

namespace Kargono::Utility
{
	Math::vec2 ScreenResolutionToVec2(Projects::ScreenResolutionOptions option)
	{
		switch (option)
		{
		case Projects::ScreenResolutionOptions::R400x400: return Math::vec2(400.0f, 400.0f);
		case Projects::ScreenResolutionOptions::R800x800: return Math::vec2(800.0f, 800.0f);

		case Projects::ScreenResolutionOptions::R1920x1080: return Math::vec2(1920.0f, 1080.0f);
		case Projects::ScreenResolutionOptions::R1600x900: return Math::vec2(1600.0f, 900.0f);
		case Projects::ScreenResolutionOptions::R1366x768: return Math::vec2(1366.0f, 768.0f);
		case Projects::ScreenResolutionOptions::R1280x720: return Math::vec2(1280.0f, 720.0f);

		case Projects::ScreenResolutionOptions::R1600x1200: return Math::vec2(1600.0f, 1200.0f);
		case Projects::ScreenResolutionOptions::R1280x960: return Math::vec2(1280.0f, 960.0f);
		case Projects::ScreenResolutionOptions::R1152x864: return Math::vec2(1152.0f, 864.0f);
		case Projects::ScreenResolutionOptions::R1024x768: return Math::vec2(1024.0f, 768.0f);

		case Projects::ScreenResolutionOptions::None: return Math::vec2(400.0f, 400.0f);
		case Projects::ScreenResolutionOptions::MatchDevice: return EngineService::GetActiveWindow().GetMonitorDimensions();

		}

		KG_ERROR("Invalid ScreenResolutionOptions enum provided to ScreenResolutionToVec2 function");
		return Math::vec2(400.0f, 400.0f);
	}

	static Math::uvec2 GetClosestAspectRatio(Math::vec2 monitorResolution)
	{

		// Converting float to uint32_t to allow use of switch statement
		constexpr uint32_t widescreen16_9{ static_cast<uint32_t>((16.0f / 9.0f) * 100) };
		constexpr uint32_t fullscreen4_3{ static_cast<uint32_t>((4.0f / 3.0f) * 100) };
		constexpr uint32_t square1_1{ 1 * 100 };

		uint32_t monitorAspectRatio = static_cast<uint32_t>((monitorResolution.x / monitorResolution.y) * 100);

		switch (monitorAspectRatio)
		{
		case widescreen16_9: return Math::uvec2(16, 9);
		case fullscreen4_3: return Math::uvec2(4, 3);
		case square1_1: return Math::uvec2(1, 1);
		}

		KG_ERROR("Unknown Screen Resolution Used");
		return Math::uvec2(1, 1);

	}

	Math::uvec2 ScreenResolutionToAspectRatio(Projects::ScreenResolutionOptions option)
	{
		switch (option)
		{

		case Projects::ScreenResolutionOptions::R800x800: return Math::uvec2(1, 1);
		case Projects::ScreenResolutionOptions::R400x400: return Math::uvec2(1, 1);

		case Projects::ScreenResolutionOptions::R1280x720:
		case Projects::ScreenResolutionOptions::R1600x900:
		case Projects::ScreenResolutionOptions::R1366x768:
		case Projects::ScreenResolutionOptions::R1920x1080: return Math::uvec2(16, 9);

		case Projects::ScreenResolutionOptions::R1600x1200:
		case Projects::ScreenResolutionOptions::R1280x960:
		case Projects::ScreenResolutionOptions::R1152x864:
		case Projects::ScreenResolutionOptions::R1024x768: return Math::uvec2(4, 3);

		case Projects::ScreenResolutionOptions::MatchDevice: return GetClosestAspectRatio(EngineService::GetActiveWindow().GetMonitorDimensions());
		case Projects::ScreenResolutionOptions::None: return Math::uvec2(1, 1);
		}
		KG_ERROR("Invalid ScreenResolutionOptions enum provided to ScreenResolutionToString function");
		return Math::uvec2(1, 1);
	}
}

namespace Kargono::Projects
{
	Ref<Project> ProjectService::s_ActiveProject { nullptr };

	void ProjectService::ExportProject(const std::filesystem::path& exportLocation, bool createServer)
	{
		KG_INFO("Beginning export project process");

		KG_ASSERT(s_ActiveProject, "Failed to export project since no active project is open!")

		std::filesystem::path exportDirectory = exportLocation / s_ActiveProject->Name;
		std::filesystem::path serverExportDirectory = exportLocation / (s_ActiveProject->Name + "Server");

		KG_INFO("Creating {} Project Directory", s_ActiveProject->Name);
		Utility::FileSystem::CreateNewDirectory(exportDirectory);

		KG_INFO("Copying Runtime resources into export directory");
		Utility::FileSystem::CreateNewDirectory(exportDirectory / "Resources");
		bool success = Utility::FileSystem::CopyDirectory(
			std::filesystem::current_path().parent_path() / "Runtime/Resources",
			exportDirectory / "Resources");
		if (!success)
		{
			KG_WARN("Failed to export project due to inability to copy runtime resources directory!");
			Utility::FileSystem::DeleteSelectedDirectory(exportDirectory);
			return;
		}

		KG_INFO("Copying {} project files into export directory", s_ActiveProject->Name);
		success = Utility::FileSystem::CopyDirectory(
			GetActiveProjectDirectory(), 
			exportDirectory);
		if (!success)
		{
			KG_WARN("Failed to export project due to inability to copy project's directory!");
			Utility::FileSystem::DeleteSelectedDirectory(exportDirectory);
			return;
		}

		KG_INFO("Copying runtime shared libraries");
		// Copy OpenAL.dll file
		success = Utility::FileSystem::CopySingleFile(
			std::filesystem::current_path().parent_path() / "Dependencies/OpenAL/lib/dist/OpenAL32.dll",
			exportDirectory / "OpenAL32.dll");
		if (!success)
		{
			KG_WARN("Failed to move OpenAL32.dll file into export directory!");
			Utility::FileSystem::DeleteSelectedDirectory(exportDirectory);
			return;
		}

		if (createServer)
		{
			KG_INFO("Creating server export directory at {}", serverExportDirectory.string());
			Utility::FileSystem::CreateNewDirectory(serverExportDirectory);
			success = Utility::FileSystem::CopyDirectory(exportDirectory, serverExportDirectory);
			if (!success)
			{
				KG_WARN("Failed to copy files from exportDirectory to serverExportDirectory");
				Utility::FileSystem::DeleteSelectedDirectory(exportDirectory);
				Utility::FileSystem::DeleteSelectedDirectory(serverExportDirectory);
				return;
			}
		}

		KG_INFO("Building runtime executable");
		Utility::FileSystem::DeleteSelectedFile("Log/BuildRuntimeExecutable.log");
		success = BuildRuntimeExecutable(exportDirectory, false);

		if (!success)
		{
			KG_WARN("Failed to build runtime executable into export directory");
			Utility::FileSystem::DeleteSelectedDirectory(exportDirectory);
			if (createServer)
			{
				Utility::FileSystem::DeleteSelectedDirectory(serverExportDirectory);
			}
			return;
		}

		if (createServer)
		{
			KG_INFO("Building server executable");
			Utility::FileSystem::DeleteSelectedFile("Log/BuildServerExecutable.log");
			success = BuildRuntimeExecutable(serverExportDirectory, true);

			if (!success)
			{
				KG_WARN("Failed to build server executable into export directory");
				Utility::FileSystem::DeleteSelectedDirectory(exportDirectory);
				Utility::FileSystem::DeleteSelectedDirectory(serverExportDirectory);
				return;
			}
		}

		KG_INFO("Successfully exported {} project to {}", s_ActiveProject->Name, exportDirectory.string());
		if (createServer)
		{
			KG_INFO("Successfully exported {} project server to {}", s_ActiveProject->Name, serverExportDirectory.string());
		}
	}
	bool ProjectService::BuildRuntimeExecutable(const std::filesystem::path& exportDirectory, bool createServer)
	{
		KG_ASSERT(s_ActiveProject, "Failed to build runtime executable since no active project exists");
		std::filesystem::path solutionPath { std::filesystem::current_path().parent_path() / "Kargono.sln" };
		std::filesystem::path intermediatesPath { exportDirectory / "Temp/" };

		// Check if the sourceFile path exists and is a regular file
		if (!std::filesystem::exists(solutionPath) || !std::filesystem::is_regular_file(solutionPath))
		{
			KG_WARN("Failed to build executable. Could not locate/invalid Kargono.sln file");
			return false;
		}

		std::stringstream outputStream {};
		outputStream << "("; // Parentheses to group all function calls together
		// Access visual studio toolset console
		outputStream << "\"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat\"";

		outputStream << " && set ExternalCompilerOptions=KG_EXPORT ";

		// Start Next Command
		outputStream << " && "; // Combine commands

		// Cl command for compiling binary code
		outputStream << "msbuild "; // Add Command

		outputStream << "\"" << solutionPath.string() << "\" "; // Provide path to solution
		if (createServer)
		{
			outputStream << "-t:Engine\\Engine,Applications\\Server "; // Specify Runtime project to be built
		}
		else
		{
			outputStream << "-t:Engine\\Engine,Applications\\Runtime "; // Specify Runtime project to be built
		}

		outputStream << "-p:OutDir=" << "\"" << intermediatesPath.string() << "/\" "; // Provide path to place executable

		// Specify the intermediate output directory
		outputStream << "-p:IntermediateOutputPath=" << "\"" << intermediatesPath.string() << "\" "; // Provide intermediate directory path

		outputStream << "-p:Configuration=Dist "; // Run Dist configuration

		// Specify the preprocessor define(s)
		outputStream << "-p:KG_EXPORT=1" << " "; // Add KG_EXPORT define

		outputStream << ")"; // Parentheses to group all function calls together

		// Sends all three calls (open dev console, compiler, and linker) error/info to log file
		if (createServer)
		{
			outputStream << " >> Log\\BuildServerExecutable.log 2>&1 ";
		}
		else
		{
			outputStream << " >> Log\\BuildRuntimeExecutable.log 2>&1 ";
		}
		

		// Call Command
		bool success = system(outputStream.str().c_str()) != 1;

		if (!success)
		{
			KG_WARN("Command line operation returned error code 1");
			return false;
		}

		// Move Executable into main directory
		if (createServer)
		{
			success = Utility::FileSystem::CopySingleFile(intermediatesPath / "Server.exe", exportDirectory / (s_ActiveProject->Name + "Server.exe"));
		}
		else
		{
			success = Utility::FileSystem::CopySingleFile(intermediatesPath / "Runtime.exe", exportDirectory / (s_ActiveProject->Name + ".exe"));
		}

		if (!success)
		{
			KG_WARN("Failed to copy executable file into export directory");
			return false;
		}

		// Remove Intermediates
		Utility::FileSystem::DeleteSelectedDirectory(intermediatesPath);

		return true;
	}
}


