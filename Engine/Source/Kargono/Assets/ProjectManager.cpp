#include "kgpch.h"

#include "Kargono/Assets/AssetManager.h"
#include "Kargono/Projects/Project.h"

#include "API/Serialization/yamlcppAPI.h"

namespace Kargono::Assets
{
	bool AssetManager::DeserializeServerVariables(Ref<Projects::Project> project, const std::filesystem::path& projectPath)
	{
		std::filesystem::path filepath = (projectPath.parent_path() / "server_variables.env");

		if (!std::filesystem::exists(filepath))
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


	std::filesystem::path AssetManager::CreateNewProject(const std::string& projectName, const std::filesystem::path& projectLocation)
	{

		Ref<Projects::Project> newProject  = CreateRef<Projects::Project>();
		newProject->Name = projectName;
		newProject->m_ProjectDirectory = projectLocation / projectName;
		newProject->AssetDirectory = "Assets";

		if (exists(newProject->m_ProjectDirectory))
		{
			KG_WARN("Attempt to create new project when same named directory already exists");
			return {};
		}
		// Create Project Directory
		Utility::FileSystem::CreateNewDirectory(newProject->m_ProjectDirectory);

		std::filesystem::path projectFileLocation = newProject->m_ProjectDirectory / (projectName + ".kproj");
		bool success = Assets::AssetManager::SerializeProject(newProject, projectFileLocation);
		if (!success)
		{
			KG_WARN("Failed to serialize project file");
			Utility::FileSystem::DeleteSelectedDirectory(newProject->m_ProjectDirectory);
			return {};
		}

		// Create all asset folders
		Utility::FileSystem::CreateNewDirectory(newProject->m_ProjectDirectory / newProject->AssetDirectory);
		Utility::FileSystem::CreateNewDirectory(newProject->m_ProjectDirectory / newProject->AssetDirectory / "Audio");
		Utility::FileSystem::CreateNewDirectory(newProject->m_ProjectDirectory / newProject->AssetDirectory / "EntityClass");
		Utility::FileSystem::CreateNewDirectory(newProject->m_ProjectDirectory / newProject->AssetDirectory / "Fonts");
		Utility::FileSystem::CreateNewDirectory(newProject->m_ProjectDirectory / newProject->AssetDirectory / "GameState");
		Utility::FileSystem::CreateNewDirectory(newProject->m_ProjectDirectory / newProject->AssetDirectory / "Input");
		Utility::FileSystem::CreateNewDirectory(newProject->m_ProjectDirectory / newProject->AssetDirectory / "Scenes");
		Utility::FileSystem::CreateNewDirectory(newProject->m_ProjectDirectory / newProject->AssetDirectory / "Scripting");
		Utility::FileSystem::CreateNewDirectory(newProject->m_ProjectDirectory / newProject->AssetDirectory / "Shaders");
		Utility::FileSystem::CreateNewDirectory(newProject->m_ProjectDirectory / newProject->AssetDirectory / "Textures");
		Utility::FileSystem::CreateNewDirectory(newProject->m_ProjectDirectory / newProject->AssetDirectory / "UserInterface");

		return projectFileLocation;
		
	}
	Ref<Projects::Project> AssetManager::OpenProject(const std::filesystem::path& path)
	{
		Ref<Projects::Project> project = CreateRef<Projects::Project>();
		if (Assets::AssetManager::DeserializeProject(project, path))
		{
			project->m_ProjectDirectory = path.parent_path();
			Projects::ProjectService::s_ActiveProject = project;
			return Projects::ProjectService::s_ActiveProject;
		}

		return nullptr;

	}
	bool AssetManager::SaveActiveProject(const std::filesystem::path& path)
	{
		if (Assets::AssetManager::SerializeProject(Projects::ProjectService::s_ActiveProject, path))
		{
			Projects::ProjectService::s_ActiveProject->m_ProjectDirectory = path.parent_path();
			return true;
		}
		return false;
	}

	bool AssetManager::SerializeProject(Ref<Projects::Project> project, const std::filesystem::path& filepath)
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

				if (project->AppTickGenerators.size() > 0)
				{
					out << YAML::Key << "AppTickGenerators" << YAML::BeginSeq;
					// Serialize App Tick Generators
					for (auto generatorValue : project->AppTickGenerators)
					{
						out << YAML::Value << generatorValue;
					}
					out << YAML::EndSeq;
				}

				out << YAML::EndMap; // Project
			}

			out << YAML::EndMap; // Root
		}

		std::ofstream fout(filepath);
		fout << out.c_str();
		KG_INFO("Successfully Serialized Project {}", project->Name);
		return true;
	}

	bool AssetManager::DeserializeProject(Ref<Projects::Project> project, const std::filesystem::path& filepath)
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
		project->StartSceneHandle = static_cast<AssetHandle>(projectNode["StartSceneHandle"].as<uint64_t>());
		project->StartGameState = static_cast<AssetHandle>(projectNode["StartGameState"].as<uint64_t>());
		project->AssetDirectory = projectNode["AssetDirectory"].as<std::string>();
		project->ScriptModulePath = projectNode["ScriptModulePath"].as<std::string>();
		project->ScriptDLLPath = projectNode["ScriptDLLPath"].as<std::string>();
		project->DefaultFullscreen = projectNode["DefaultFullscreen"].as<bool>();
		project->TargetResolution = Utility::StringToScreenResolution(projectNode["TargetResolution"].as<std::string>());
		project->OnRuntimeStart = static_cast<AssetHandle>(projectNode["OnRuntimeStart"].as<uint64_t>());
		project->OnUpdateUserCount = static_cast<AssetHandle>(projectNode["OnUpdateUserCount"].as<uint64_t>());
		project->OnApproveJoinSession = static_cast<AssetHandle>(projectNode["OnApproveJoinSession"].as<uint64_t>());
		project->OnUserLeftSession = static_cast<AssetHandle>(projectNode["OnUserLeftSession"].as<uint64_t>());
		project->OnCurrentSessionInit = static_cast<AssetHandle>(projectNode["OnCurrentSessionInit"].as<uint64_t>());
		project->OnConnectionTerminated = static_cast<AssetHandle>(projectNode["OnConnectionTerminated"].as<uint64_t>());
		project->OnUpdateSessionUserSlot = static_cast<AssetHandle>(projectNode["OnUpdateSessionUserSlot"].as<uint64_t>());
		project->OnStartSession = static_cast<AssetHandle>(projectNode["OnStartSession"].as<uint64_t>());
		project->OnSessionReadyCheckConfirm = static_cast<AssetHandle>(projectNode["OnSessionReadyCheckConfirm"].as<uint64_t>());
		project->OnReceiveSignal = static_cast<AssetHandle>(projectNode["OnReceiveSignal"].as<uint64_t>());
		project->AppIsNetworked = static_cast<AssetHandle>(projectNode["AppIsNetworked"].as<bool>());

		auto tickGenerators = projectNode["AppTickGenerators"];

		if (tickGenerators)
		{
			for (auto generator : tickGenerators)
			{
				uint64_t value = generator.as<uint64_t>();
				project->AppTickGenerators.insert(value);
			}
		}

		DeserializeServerVariables(project, filepath);

		return true;
	}

	//===================================================================================================================================================
}
