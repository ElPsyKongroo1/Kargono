#include "kgpch.h"

#include "Kargono/Projects/Project.h"
#include "Kargono/Core/Engine.h"
#include "Kargono/Assets/Asset.h"

#include "API/Serialization/yamlcppAPI.h"

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

	bool ProjectService::DeserializeServerVariables(Ref<Projects::Project> project, const std::filesystem::path& projectPath)
	{
		std::filesystem::path filepath = (projectPath.parent_path() / "server_variables.env");

		if (!Utility::FileSystem::PathExists(filepath))
		{
			KG_WARN("No server_variables.env file found. Default settings applied.");
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
		auto rootNode = data["ServerVariables"];
		if (!rootNode) { return false; }

		project->ServerIP = rootNode["ServerIP"].as<std::string>();
		project->ServerPort = static_cast<uint16_t>(rootNode["ServerPort"].as<uint32_t>());
		project->ServerLocation = rootNode["ServerLocation"].as<std::string>();
		project->SecretOne = rootNode["SecretOne"].as<uint64_t>();
		project->SecretTwo = rootNode["SecretTwo"].as<uint64_t>();
		project->SecretThree = rootNode["SecretThree"].as<uint64_t>();
		project->SecretFour = rootNode["SecretFour"].as<uint64_t>();

		return true;
	}


	std::filesystem::path ProjectService::CreateNewProject(const std::string& projectName, const std::filesystem::path& projectLocation)
	{

		Ref<Projects::Project> newProject = CreateRef<Projects::Project>();
		newProject->Name = projectName;
		newProject->m_ProjectDirectory = projectLocation / projectName;
		newProject->AssetDirectory = "Assets";
		newProject->IntermediateDirectory = "Intermediates";

		if (exists(newProject->m_ProjectDirectory))
		{
			KG_WARN("Attempt to create new project when same named directory already exists");
			return {};
		}
		// Create Project Directory
		Utility::FileSystem::CreateNewDirectory(newProject->m_ProjectDirectory);

		std::filesystem::path projectFileLocation = newProject->m_ProjectDirectory / (projectName + ".kproj");
		bool success = SerializeProject(newProject, projectFileLocation);
		if (!success)
		{
			KG_WARN("Failed to serialize project file");
			Utility::FileSystem::DeleteSelectedDirectory(newProject->m_ProjectDirectory);
			return {};
		}

		// Create all asset folders
		Utility::FileSystem::CreateNewDirectory(newProject->m_ProjectDirectory / newProject->AssetDirectory);
		Utility::FileSystem::CreateNewDirectory(newProject->m_ProjectDirectory / newProject->IntermediateDirectory);
		Utility::FileSystem::CreateNewDirectory(newProject->m_ProjectDirectory / newProject->IntermediateDirectory / "AudioBuffer");
		Utility::FileSystem::CreateNewDirectory(newProject->m_ProjectDirectory / newProject->IntermediateDirectory / "EntityClass");
		Utility::FileSystem::CreateNewDirectory(newProject->m_ProjectDirectory / newProject->IntermediateDirectory / "Font");
		Utility::FileSystem::CreateNewDirectory(newProject->m_ProjectDirectory / newProject->IntermediateDirectory / "GameState");
		Utility::FileSystem::CreateNewDirectory(newProject->m_ProjectDirectory / newProject->IntermediateDirectory / "Input");
		Utility::FileSystem::CreateNewDirectory(newProject->m_ProjectDirectory / newProject->IntermediateDirectory / "Scene");
		Utility::FileSystem::CreateNewDirectory(newProject->m_ProjectDirectory / newProject->IntermediateDirectory / "Script");
		Utility::FileSystem::CreateNewDirectory(newProject->m_ProjectDirectory / newProject->IntermediateDirectory / "Shader");
		Utility::FileSystem::CreateNewDirectory(newProject->m_ProjectDirectory / newProject->IntermediateDirectory / "Texture2D");
		Utility::FileSystem::CreateNewDirectory(newProject->m_ProjectDirectory / newProject->IntermediateDirectory / "UserInterface");

		return projectFileLocation;

	}
	Ref<Projects::Project> ProjectService::OpenProject(const std::filesystem::path& path)
	{
		Ref<Projects::Project> project = CreateRef<Projects::Project>();
		if (DeserializeProject(project, path))
		{
			project->m_ProjectDirectory = path.parent_path();
			s_ActiveProject = project;
			return s_ActiveProject;
		}

		return nullptr;

	}

	bool ProjectService::SaveActiveProject()
	{
		return Projects::ProjectService::SaveActiveProject((Projects::ProjectService::GetActiveProjectDirectory() / Projects::ProjectService::GetActiveProjectName()).replace_extension(".kproj"));
	}

	bool ProjectService::SaveActiveProject(const std::filesystem::path& path)
	{
		if (SerializeProject(s_ActiveProject, path))
		{
			s_ActiveProject->m_ProjectDirectory = path.parent_path();
			return true;
		}
		return false;
	}

	bool ProjectService::SerializeProject(Ref<Projects::Project> project, const std::filesystem::path& filepath)
	{
		YAML::Emitter out;

		{
			out << YAML::BeginMap; // Root
			out << YAML::Key << "Project" << YAML::Value;
			{
				out << YAML::BeginMap; // Project
				out << YAML::Key << "Name" << YAML::Value << project->Name;
				out << YAML::Key << "StartSceneHandle" << YAML::Value << static_cast<uint64_t>(project->StartSceneHandle);
				out << YAML::Key << "StartGameState" << YAML::Value << static_cast<uint64_t>(project->StartGameState);
				out << YAML::Key << "AssetDirectory" << YAML::Value << project->AssetDirectory.string();
				out << YAML::Key << "IntermediateDirectory" << YAML::Value << project->IntermediateDirectory.string();
				out << YAML::Key << "ScriptModulePath" << YAML::Value << project->ScriptModulePath.string();
				out << YAML::Key << "ScriptDLLPath" << YAML::Value << project->ScriptDLLPath.string();
				out << YAML::Key << "DefaultFullscreen" << YAML::Value << project->DefaultFullscreen;
				out << YAML::Key << "TargetResolution" << YAML::Value << Utility::ScreenResolutionToString(project->TargetResolution);
				out << YAML::Key << "OnRuntimeStart" << YAML::Value << static_cast<uint64_t>(project->OnRuntimeStart);
				out << YAML::Key << "OnUpdateUserCount" << YAML::Value << static_cast<uint64_t>(project->OnUpdateUserCount);
				out << YAML::Key << "OnApproveJoinSession" << YAML::Value << static_cast<uint64_t>(project->OnApproveJoinSession);
				out << YAML::Key << "OnUserLeftSession" << YAML::Value << static_cast<uint64_t>(project->OnUserLeftSession);
				out << YAML::Key << "OnCurrentSessionInit" << YAML::Value << static_cast<uint64_t>(project->OnCurrentSessionInit);
				out << YAML::Key << "OnConnectionTerminated" << YAML::Value << static_cast<uint64_t>(project->OnConnectionTerminated);
				out << YAML::Key << "OnUpdateSessionUserSlot" << YAML::Value << static_cast<uint64_t>(project->OnUpdateSessionUserSlot);
				out << YAML::Key << "OnStartSession" << YAML::Value << static_cast<uint64_t>(project->OnStartSession);
				out << YAML::Key << "OnSessionReadyCheckConfirm" << YAML::Value << (uint64_t)project->OnSessionReadyCheckConfirm;
				out << YAML::Key << "OnReceiveSignal" << YAML::Value << (uint64_t)project->OnReceiveSignal;
				out << YAML::Key << "AppIsNetworked" << YAML::Value << project->AppIsNetworked;

				// TODO: Remove App Tick Generators
				// Serialize App Tick Generators
				if (project->AppTickGenerators.size() > 0)
				{
					out << YAML::Key << "AppTickGenerators" << YAML::BeginSeq;
					for (auto generatorValue : project->AppTickGenerators)
					{
						out << YAML::Value << generatorValue;
					}
					out << YAML::EndSeq;
				}

				// Serialize AI Message Types
				out << YAML::Key << "AllMessageTypes" << YAML::BeginSeq;
				for (std::string& aiTypeName : project->AllMessageTypes)
				{
					out << YAML::Value << aiTypeName;
				}
				out << YAML::EndSeq;

				out << YAML::EndMap; // Close Project
			}

			out << YAML::EndMap; // Root
		}

		std::ofstream fout(filepath);
		fout << out.c_str();
		KG_INFO("Successfully Serialized Project {}", project->Name);
		return true;
	}

	bool ProjectService::DeserializeProject(Ref<Projects::Project> project, const std::filesystem::path& filepath)
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
		auto projectNode = data["Project"];
		if (!projectNode) { return false; }

		project->Name = projectNode["Name"].as<std::string>();
		project->StartSceneHandle = static_cast<Assets::AssetHandle>(projectNode["StartSceneHandle"].as<uint64_t>());
		project->StartGameState = static_cast<Assets::AssetHandle>(projectNode["StartGameState"].as<uint64_t>());
		project->AssetDirectory = projectNode["AssetDirectory"].as<std::string>();
		project->IntermediateDirectory = projectNode["IntermediateDirectory"].as<std::string>();
		project->ScriptModulePath = projectNode["ScriptModulePath"].as<std::string>();
		project->ScriptDLLPath = projectNode["ScriptDLLPath"].as<std::string>();
		project->DefaultFullscreen = projectNode["DefaultFullscreen"].as<bool>();
		project->TargetResolution = Utility::StringToScreenResolution(projectNode["TargetResolution"].as<std::string>());
		project->OnRuntimeStart = static_cast<Assets::AssetHandle>(projectNode["OnRuntimeStart"].as<uint64_t>());
		project->OnUpdateUserCount = static_cast<Assets::AssetHandle>(projectNode["OnUpdateUserCount"].as<uint64_t>());
		project->OnApproveJoinSession = static_cast<Assets::AssetHandle>(projectNode["OnApproveJoinSession"].as<uint64_t>());
		project->OnUserLeftSession = static_cast<Assets::AssetHandle>(projectNode["OnUserLeftSession"].as<uint64_t>());
		project->OnCurrentSessionInit = static_cast<Assets::AssetHandle>(projectNode["OnCurrentSessionInit"].as<uint64_t>());
		project->OnConnectionTerminated = static_cast<Assets::AssetHandle>(projectNode["OnConnectionTerminated"].as<uint64_t>());
		project->OnUpdateSessionUserSlot = static_cast<Assets::AssetHandle>(projectNode["OnUpdateSessionUserSlot"].as<uint64_t>());
		project->OnStartSession = static_cast<Assets::AssetHandle>(projectNode["OnStartSession"].as<uint64_t>());
		project->OnSessionReadyCheckConfirm = static_cast<Assets::AssetHandle>(projectNode["OnSessionReadyCheckConfirm"].as<uint64_t>());
		project->OnReceiveSignal = static_cast<Assets::AssetHandle>(projectNode["OnReceiveSignal"].as<uint64_t>());
		project->AppIsNetworked = static_cast<Assets::AssetHandle>(projectNode["AppIsNetworked"].as<bool>());

		YAML::Node tickGenerators = projectNode["AppTickGenerators"];

		if (tickGenerators)
		{
			for (auto generator : tickGenerators)
			{
				uint64_t value = generator.as<uint64_t>();
				project->AppTickGenerators.insert(value);
			}
		}

		YAML::Node aiMessageTypeNode = projectNode["AllMessageTypes"];
		std::vector<std::string>& allMessageTypesRef = project->AllMessageTypes;
		for (auto aiMessage : aiMessageTypeNode)
		{
			allMessageTypesRef.push_back(aiMessage.as<std::string>());
		}
		
		DeserializeServerVariables(project, filepath);

		return true;
	}

	bool ProjectService::RemoveScriptFromActiveProject(Assets::AssetHandle scriptHandle)
	{
		bool projectModified{ false };
		// Check active project for scripts
		if (Projects::ProjectService::GetActiveOnRuntimeStartHandle() == scriptHandle)
		{
			Projects::ProjectService::SetActiveOnRuntimeStartHandle(Assets::EmptyHandle);
			projectModified = true;
		}
		if (Projects::ProjectService::GetActiveOnUpdateUserCountHandle() == scriptHandle)
		{
			Projects::ProjectService::SetActiveOnUpdateUserCountHandle(Assets::EmptyHandle);
			projectModified = true;
		}
		if (Projects::ProjectService::GetActiveOnApproveJoinSessionHandle() == scriptHandle)
		{
			Projects::ProjectService::SetActiveOnApproveJoinSessionHandle(Assets::EmptyHandle);
			projectModified = true;
		}
		if (Projects::ProjectService::GetActiveOnUserLeftSessionHandle() == scriptHandle)
		{
			Projects::ProjectService::SetActiveOnUserLeftSessionHandle(Assets::EmptyHandle);
			projectModified = true;
		}
		if (Projects::ProjectService::GetActiveOnCurrentSessionInitHandle() == scriptHandle)
		{
			Projects::ProjectService::SetActiveOnCurrentSessionInitHandle(Assets::EmptyHandle);
			projectModified = true;
		}
		if (Projects::ProjectService::GetActiveOnConnectionTerminatedHandle() == scriptHandle)
		{
			Projects::ProjectService::SetActiveOnConnectionTerminatedHandle(Assets::EmptyHandle);
			projectModified = true;
		}
		if (Projects::ProjectService::GetActiveOnUpdateSessionUserSlotHandle() == scriptHandle)
		{
			Projects::ProjectService::SetActiveOnUpdateSessionUserSlotHandle(Assets::EmptyHandle);
			projectModified = true;
		}
		if (Projects::ProjectService::GetActiveOnStartSessionHandle() == scriptHandle)
		{
			Projects::ProjectService::SetActiveOnStartSessionHandle(Assets::EmptyHandle);
			projectModified = true;
		}
		if (Projects::ProjectService::GetActiveOnSessionReadyCheckConfirmHandle() == scriptHandle)
		{
			Projects::ProjectService::SetActiveOnSessionReadyCheckConfirmHandle(Assets::EmptyHandle);
			projectModified = true;
		}
		if (Projects::ProjectService::GetActiveOnReceiveSignalHandle() == scriptHandle)
		{
			Projects::ProjectService::SetActiveOnReceiveSignalHandle(Assets::EmptyHandle);
			projectModified = true;
		}

		return projectModified;
	}

	
}


