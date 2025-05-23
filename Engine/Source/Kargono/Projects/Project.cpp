#include "kgpch.h"

#include "Kargono/Projects/Project.h"
#include "Modules/Core/Engine.h"
#include "Modules/Assets/Asset.h"
#include "Modules/Network/NetworkTools.h"

#include "API/Serialization/yamlcppAPI.h"

#if defined(KG_PLATFORM_WINDOWS) 
#include "API/Platform/WindowsBackendAPI.h"
#elif defined(KG_PLATFORM_LINUX)
#include "API/Platform/LinuxBackendAPI.h"
#endif

namespace Kargono::Projects
{
	void Project::ExportProject(const std::filesystem::path& exportLocation, bool createServer)
	{
		KG_INFO("Beginning export project process");

		// Create full path to the export directories of the runtime and server
		std::filesystem::path exportDirectory = exportLocation / m_Name;
		std::filesystem::path serverExportDirectory = exportLocation / (m_Name + "Server");

		KG_INFO("Creating {} Project Directory", m_Name);
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

		KG_INFO("Copying {} project files into export directory", m_Name);
		success = Utility::FileSystem::CopyDirectory(
			GetProjectPaths().m_ProjectDirectory,
			exportDirectory);
		if (!success)
		{
			KG_WARN("Failed to export project due to inability to copy project's directory!");
			Utility::FileSystem::DeleteSelectedDirectory(exportDirectory);
			return;
		}

		KG_INFO("Copying runtime shared libraries");
		// Copy OpenAL shared library
#if defined(KG_PLATFORM_WINDOWS)
		success = Utility::FileSystem::CopySingleFile(
			std::filesystem::current_path().parent_path() / "Dependencies/OpenAL/lib/dist/OpenAL32.dll",
			exportDirectory / "OpenAL32.dll");
#elif defined(KG_PLATFORM_LINUX)
		success = Utility::FileSystem::CopySingleFile(
			std::filesystem::current_path().parent_path() / "Dependencies/OpenAL/lib/dist/libopenal.so",
			exportDirectory / "libopenal.so");
#endif
		if (!success)
		{
			KG_WARN("Failed to move openAL shared library file into export directory!");
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
#if defined(KG_PLATFORM_WINDOWS)
		success = BuildExecutableMSVC(exportDirectory, false);
#elif defined(KG_PLATFORM_LINUX)
		success = BuildExecutableGCC(exportDirectory, false);
#endif
		

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
#if defined(KG_PLATFORM_WINDOWS)
				success = BuildExecutableMSVC(serverExportDirectory, true);
#elif defined(KG_PLATFORM_LINUX)
				success = BuildExecutableGCC(serverExportDirectory, true);
#endif

			if (!success)
			{
				KG_WARN("Failed to build server executable into export directory");
				Utility::FileSystem::DeleteSelectedDirectory(exportDirectory);
				Utility::FileSystem::DeleteSelectedDirectory(serverExportDirectory);
				return;
			}
		}

		KG_INFO("Successfully exported {} project to {}", m_Name, exportDirectory.string());
		if (createServer)
		{
			KG_INFO("Successfully exported {} project server to {}", m_Name, serverExportDirectory.string());
		}
	}

	bool Project::BuildExecutableGCC(const std::filesystem::path& exportDirectory, bool createServer)
	{
		std::filesystem::path projectPath { std::filesystem::current_path().parent_path()};
		std::filesystem::path intermediatesPath { exportDirectory / "Temp/" };

		// Ensure all relevant directories have been created
		Utility::FileSystem::CreateNewDirectory(intermediatesPath);
		Utility::FileSystem::CreateNewDirectory(projectPath / "Editor/Log");

		// Check if the project path exists
		if (!Utility::FileSystem::PathExists(projectPath) || !Utility::FileSystem::IsRegularFile(projectPath / "Makefile")) 
		{
			KG_WARN("Failed to build executable. Could not locate/invalid Makefile");
			return false;
		}

		std::stringstream outputStream {};

		// Group commands together
		outputStream << "{ ";

		// Change directory to the project path
		outputStream << "cd \"" << projectPath.string() << "\" && ";

		// Set environment variables or build options
		if (createServer) 
		{
			outputStream << "export ExternalCompilerOptions=KG_EXPORT_SERVER && ";
		} 
		else 
		{
			outputStream << "export ExternalCompilerOptions=KG_EXPORT_RUNTIME && ";
		}

		// Specify make command
		outputStream << "make ";

		// Specify target executable
		if (createServer)
		{
			outputStream << "Server ";
		}
		else
		{
			outputStream << "Runtime ";
		}

		// Specify the output directory and configuration
		outputStream << "TARGETDIR=\"" << intermediatesPath.string() << "\" ";
		outputStream << "OBJDIR=\"" << intermediatesPath.string() << "\" ";
		outputStream << "CONFIG=Dist; ";

		// Group commands together
		outputStream << " } ";

		// Redirect output to log files
		if (createServer) 
		{
			outputStream << ">> ./Log/BuildServerExecutable.log 2>&1 ";
		} 
		else 
		{
			outputStream << ">> ./Log/BuildRuntimeExecutable.log 2>&1 ";
		}
		// Execute the build command
		bool success = system(outputStream.str().c_str()) == 0;

		if (!success) 
		{
			KG_WARN("Command line operation returned error code");
			return false;
		}

		// Move Executable into main directory
		if (createServer) 
		{
			success = Utility::FileSystem::CopySingleFile(intermediatesPath / "Server", exportDirectory / (m_Name + "Server"));
		} 
		else 
		{
			success = Utility::FileSystem::CopySingleFile(intermediatesPath / "Runtime", exportDirectory / (m_Name));
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

	bool Project::BuildExecutableMSVC(const std::filesystem::path& exportDirectory, bool createServer)
	{
		std::filesystem::path solutionPath { std::filesystem::current_path().parent_path() / "Kargono.sln" };
		std::filesystem::path intermediatesPath { exportDirectory / "Temp/" };

		// Check if the sourceFile path exists and is a regular file
		if (!Utility::FileSystem::PathExists(solutionPath) || !Utility::FileSystem::IsRegularFile(solutionPath))
		{
			KG_WARN("Failed to build executable. Could not locate/invalid Kargono.sln file");
			return false;
		}

		std::stringstream outputStream {};
		outputStream << "("; // Parentheses to group all function calls together
		// Access visual studio toolset console
		outputStream << "\"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvars64.bat\"";

		if (createServer)
		{
			outputStream << " && set ExternalCompilerOptions=KG_EXPORT_SERVER ";
		}
		else
		{
			outputStream << " && set ExternalCompilerOptions=KG_EXPORT_RUNTIME ";
		}

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
		if (createServer)
		{
			outputStream << "-p:KG_EXPORT_SERVER=1" << " "; // Add KG_EXPORT define
		}
		else
		{
			outputStream << "-p:KG_EXPORT_RUNTIME=1" << " "; // Add KG_EXPORT define
		}

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
			success = Utility::FileSystem::CopySingleFile(intermediatesPath / "Server.exe", exportDirectory / (m_Name + "Server.exe"));
		}
		else
		{
			success = Utility::FileSystem::CopySingleFile(intermediatesPath / "Runtime.exe", exportDirectory / (m_Name + ".exe"));
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

	bool Project::SerializeServerConfig()
	{
		// Create/overwrite the server config file if specified
		bool writeConfigError{ false };

		// Create the config file path
		std::filesystem::path configFilePath = m_ProjectPaths.m_ProjectDirectory / "ServerConfig.env";
		KG_INFO("Creating server config file at {}", configFilePath.string());

		// Write the config file path out to disk (project location)
		std::string newConfigFile = Network::NetworkTools::CreateServerVariablesConfigFile(m_ServerConfig);
		writeConfigError = !Utility::FileSystem::WriteFileString(configFilePath, newConfigFile);

		// Ensure config file creation does not throw errors
		if (writeConfigError)
		{
			KG_WARN("Error occurred while writing config file to disk.");
			return false;
		}

		return true;
	}

	bool Project::DeserializeServerConfig(const std::filesystem::path& projectPath)
	{
		std::filesystem::path filepath = (projectPath.parent_path() / "ServerConfig.env");

		if (!Utility::FileSystem::PathExists(filepath))
		{
			KG_WARN("No ServerConfig.env file found. Default settings applied.");
			return false;
		}

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load server variables '{0}'\n     {1}", filepath, e.what());
			return false;
		}
		YAML::Node rootNode = data["ServerVariables"];
		if (!rootNode) { return false; }

		Math::u8vec4 ipv4 = (Math::u8vec4)rootNode["ServerIP"].as<Math::ivec4>();
		m_ServerConfig.m_ServerAddress.SetAddress(ipv4.x, ipv4.y, ipv4.z, ipv4.w);
		m_ServerConfig.m_ServerAddress.SetNewPort(static_cast<uint16_t>(rootNode["ServerPort"].as<uint32_t>()));
		m_ServerConfig.m_ServerLocation = Utility::StringToServerLocation(rootNode["ServerLocation"].as<std::string>());

		m_ServerConfig.m_ValidationSecrets.x = rootNode["SecretOne"].as<uint64_t>();
		m_ServerConfig.m_ValidationSecrets.y = rootNode["SecretTwo"].as<uint64_t>();
		m_ServerConfig.m_ValidationSecrets.z = rootNode["SecretThree"].as<uint64_t>();
		m_ServerConfig.m_ValidationSecrets.w = rootNode["SecretFour"].as<uint64_t>();
		return true;
	}


	bool Project::CreateNewProject(const std::string& projectName, const std::filesystem::path& projectLocation)
	{
		m_Active = true;
		m_Name = projectName;
		m_ProjectPaths.m_ProjectDirectory = projectLocation / projectName;

		if (std::filesystem::exists(m_ProjectPaths.m_ProjectDirectory))
		{
			KG_WARN("Attempt to create new project when same named directory already exists");
			return false;
		}
		// Create Project Directory
		Utility::FileSystem::CreateNewDirectory(m_ProjectPaths.m_ProjectDirectory);

		std::filesystem::path projectFileLocation = m_ProjectPaths.m_ProjectDirectory / (projectName + ".kproj");

		if (!SerializeProject(projectFileLocation))
		{
			KG_WARN("Failed to serialize project file");
			Utility::FileSystem::DeleteSelectedDirectory(m_ProjectPaths.m_ProjectDirectory);
			return false;
		}

		// Create all asset folders
		Utility::FileSystem::CreateNewDirectory(m_ProjectPaths.GetAssetDirectory());
		Utility::FileSystem::CreateNewDirectory(m_ProjectPaths.GetIntermediateDirectory());
		Utility::FileSystem::CreateNewDirectory(m_ProjectPaths.GetIntermediateDirectory() / "AudioBuffer");
		Utility::FileSystem::CreateNewDirectory(m_ProjectPaths.GetIntermediateDirectory() / "EntityClass");
		Utility::FileSystem::CreateNewDirectory(m_ProjectPaths.GetIntermediateDirectory() / "Font");
		Utility::FileSystem::CreateNewDirectory(m_ProjectPaths.GetIntermediateDirectory() / "GameState");
		Utility::FileSystem::CreateNewDirectory(m_ProjectPaths.GetIntermediateDirectory() / "Input");
		Utility::FileSystem::CreateNewDirectory(m_ProjectPaths.GetIntermediateDirectory() / "Scene");
		Utility::FileSystem::CreateNewDirectory(m_ProjectPaths.GetIntermediateDirectory() / "Script");
		Utility::FileSystem::CreateNewDirectory(m_ProjectPaths.GetIntermediateDirectory() / "Shader");
		Utility::FileSystem::CreateNewDirectory(m_ProjectPaths.GetIntermediateDirectory() / "Texture2D");
		Utility::FileSystem::CreateNewDirectory(m_ProjectPaths.GetIntermediateDirectory() / "UserInterface");

		return true;

	}
	bool Project::OpenProject(const std::filesystem::path& path)
	{
		if (!DeserializeProject(path))
		{
			return false;
		}
		
		m_ProjectPaths.m_ProjectDirectory = path.parent_path();
		m_Active = true;
		return true;
	}

	bool Project::SaveProject()
	{
		return Projects::Project::SaveProject((GetProjectPaths().m_ProjectDirectory / GetProjectName()).replace_extension(".kproj"));
	}

	bool Project::SaveProject(const std::filesystem::path& path)
	{
		if (SerializeProject(path))
		{
			m_ProjectPaths.m_ProjectDirectory = path.parent_path();
			return true;
		}
		return false;
	}

	bool Project::SerializeProject(const std::filesystem::path& filepath)
	{
		YAML::Emitter out;

		{
			out << YAML::BeginMap; // Root
			out << YAML::Key << "Project" << YAML::Value;
			{
				out << YAML::BeginMap; // Project
				out << YAML::Key << "Name" << YAML::Value << m_Name;
				out << YAML::Key << "StartSceneHandle" << YAML::Value << static_cast<uint64_t>(m_StartSceneHandle);
				out << YAML::Key << "StartGameState" << YAML::Value << static_cast<uint64_t>(m_StartGameState);
				out << YAML::Key << "DefaultFullscreen" << YAML::Value << m_DefaultFullscreen;
				out << YAML::Key << "TargetResolution" << YAML::Value << Utility::ScreenResolutionToString(m_TargetResolution);
				out << YAML::Key << "OnRuntimeStart" << YAML::Value << static_cast<uint64_t>(m_OnRuntimeStart);
				out << YAML::Key << "OnUpdateUserCount" << YAML::Value << static_cast<uint64_t>(GetClientScripts().m_OnUpdateUserCount);
				out << YAML::Key << "OnApproveJoinSession" << YAML::Value << static_cast<uint64_t>(GetClientScripts().m_OnApproveJoinSession);
				out << YAML::Key << "OnUserLeftSession" << YAML::Value << static_cast<uint64_t>(GetClientScripts().m_OnUserLeftSession);
				out << YAML::Key << "OnCurrentSessionInit" << YAML::Value << static_cast<uint64_t>(GetClientScripts().m_OnCurrentSessionInit);
				out << YAML::Key << "OnConnectionTerminated" << YAML::Value << static_cast<uint64_t>(GetClientScripts().m_OnConnectionTerminated);
				out << YAML::Key << "OnUpdateSessionUserSlot" << YAML::Value << static_cast<uint64_t>(GetClientScripts().m_OnUpdateSessionUserSlot);
				out << YAML::Key << "OnStartSession" << YAML::Value << static_cast<uint64_t>(GetClientScripts().m_OnStartSession);
				out << YAML::Key << "OnSessionReadyCheckConfirm" << YAML::Value << (uint64_t)GetClientScripts().m_OnSessionReadyCheckConfirm;
				out << YAML::Key << "OnReceiveSignal" << YAML::Value << (uint64_t)GetClientScripts().m_OnReceiveSignal;
				out << YAML::Key << "AppIsNetworked" << YAML::Value << m_AppIsNetworked;

				out << YAML::EndMap; // Close Project
			}

			out << YAML::EndMap; // Root
		}

		std::ofstream fout(filepath);
		fout << out.c_str();

		KG_INFO("Successfully serialized project file {}", m_Name);

		if (SerializeServerConfig())
		{
			KG_INFO("Successfully serialized server config file");
			return true;
		}
		else
		{
			KG_WARN("Failed to serialize server config file");
			return false;
		}
	}

	bool Project::DeserializeProject(const std::filesystem::path& filepath)
	{

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load project file '{0}'\n     {1}", filepath, e.what());
			return false;
		}
		YAML::Node projectNode = data["Project"];
		if (!projectNode) { return false; }

		m_Name = projectNode["Name"].as<std::string>();
		m_StartSceneHandle = static_cast<Assets::AssetHandle>(projectNode["StartSceneHandle"].as<uint64_t>());
		m_StartGameState = static_cast<Assets::AssetHandle>(projectNode["StartGameState"].as<uint64_t>());
		m_DefaultFullscreen = projectNode["DefaultFullscreen"].as<bool>();
		m_TargetResolution = Utility::StringToScreenResolution(projectNode["TargetResolution"].as<std::string>());
		m_OnRuntimeStart = static_cast<Assets::AssetHandle>(projectNode["OnRuntimeStart"].as<uint64_t>());
		GetClientScripts().m_OnUpdateUserCount = static_cast<Assets::AssetHandle>(projectNode["OnUpdateUserCount"].as<uint64_t>());
		GetClientScripts().m_OnApproveJoinSession = static_cast<Assets::AssetHandle>(projectNode["OnApproveJoinSession"].as<uint64_t>());
		GetClientScripts().m_OnUserLeftSession = static_cast<Assets::AssetHandle>(projectNode["OnUserLeftSession"].as<uint64_t>());
		GetClientScripts().m_OnCurrentSessionInit = static_cast<Assets::AssetHandle>(projectNode["OnCurrentSessionInit"].as<uint64_t>());
		GetClientScripts().m_OnConnectionTerminated = static_cast<Assets::AssetHandle>(projectNode["OnConnectionTerminated"].as<uint64_t>());
		GetClientScripts().m_OnUpdateSessionUserSlot = static_cast<Assets::AssetHandle>(projectNode["OnUpdateSessionUserSlot"].as<uint64_t>());
		GetClientScripts().m_OnStartSession = static_cast<Assets::AssetHandle>(projectNode["OnStartSession"].as<uint64_t>());
		GetClientScripts().m_OnSessionReadyCheckConfirm = static_cast<Assets::AssetHandle>(projectNode["OnSessionReadyCheckConfirm"].as<uint64_t>());
		GetClientScripts().m_OnReceiveSignal = static_cast<Assets::AssetHandle>(projectNode["OnReceiveSignal"].as<uint64_t>());
		m_AppIsNetworked = static_cast<Assets::AssetHandle>(projectNode["AppIsNetworked"].as<bool>());
		
		DeserializeServerConfig(filepath);

		return true;
	}

	bool Project::RemoveScriptFromActiveProject(Assets::AssetHandle scriptHandle)
	{
		bool projectModified{ false };
		// Check active project for scripts
		if (GetOnRuntimeStartHandle() == scriptHandle)
		{
			SetOnRuntimeStartHandle(Assets::EmptyHandle);
			projectModified = true;
		}

		// Handle removing client scripts
		Network::ClientScripts& clientScripts{ GetClientScripts() };
		if (clientScripts.m_OnUpdateUserCount == scriptHandle)
		{
			clientScripts.m_OnUpdateUserCount = Assets::EmptyHandle;
			projectModified = true;
		}
		if (clientScripts.m_OnApproveJoinSession == scriptHandle)
		{
			clientScripts.m_OnApproveJoinSession = Assets::EmptyHandle;
			projectModified = true;
		}
		if (clientScripts.m_OnUserLeftSession == scriptHandle)
		{
			clientScripts.m_OnUserLeftSession = Assets::EmptyHandle;
			projectModified = true;
		}
		if (clientScripts.m_OnCurrentSessionInit == scriptHandle)
		{
			clientScripts.m_OnCurrentSessionInit = Assets::EmptyHandle;
			projectModified = true;
		}
		if (clientScripts.m_OnConnectionTerminated == scriptHandle)
		{
			clientScripts.m_OnConnectionTerminated = Assets::EmptyHandle;
			projectModified = true;
		}
		if (clientScripts.m_OnUpdateSessionUserSlot == scriptHandle)
		{
			clientScripts.m_OnUpdateSessionUserSlot = Assets::EmptyHandle;
			projectModified = true;
		}
		if (clientScripts.m_OnStartSession == scriptHandle)
		{
			clientScripts.m_OnStartSession = Assets::EmptyHandle;
			projectModified = true;
		}
		if (clientScripts.m_OnSessionReadyCheckConfirm == scriptHandle)
		{
			clientScripts.m_OnSessionReadyCheckConfirm = Assets::EmptyHandle;
			projectModified = true;
		}
		if (clientScripts.m_OnReceiveSignal == scriptHandle)
		{
			clientScripts.m_OnReceiveSignal = Assets::EmptyHandle;
			projectModified = true;
		}

		return projectModified;
	}
}


