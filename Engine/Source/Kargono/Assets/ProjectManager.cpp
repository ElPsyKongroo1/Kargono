#include "kgpch.h"

#include "Kargono/Assets/AssetManager.h"
#include "Kargono/Projects/Project.h"

#include "API/Serialization/yamlcppAPI.h"

namespace Kargono::Assets
{
	bool AssetManager::DeserializeServerVariables(Ref<Projects::Project> project, const std::filesystem::path& projectPath)
	{
		auto& config = project->m_Config;

		std::filesystem::path filepath = (projectPath.parent_path() / "server_variables.env");

		if (!std::filesystem::exists(filepath))
		{
			KG_WARN("Not server_variables.env file found. Default settings applied.");
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

		config.ServerIP = rootNode["ServerIP"].as<std::string>();
		config.ServerPort = static_cast<uint16_t>(rootNode["ServerPort"].as<uint32_t>());
		config.ServerLocation = rootNode["ServerLocation"].as<std::string>();
		config.SecretOne = rootNode["SecretOne"].as<uint64_t>();
		config.SecretTwo = rootNode["SecretTwo"].as<uint64_t>();
		config.SecretThree = rootNode["SecretThree"].as<uint64_t>();
		config.SecretFour = rootNode["SecretFour"].as<uint64_t>();

		return true;
	}


	Ref<Projects::Project> AssetManager::NewProject()
	{
		Projects::Project::s_ActiveProject = CreateRef<Projects::Project>();
		return Projects::Project::s_ActiveProject;
	}
	Ref<Projects::Project> AssetManager::OpenProject(const std::filesystem::path& path)
	{
		Ref<Projects::Project> project = CreateRef<Projects::Project>();
		if (Assets::AssetManager::DeserializeProject(project, path))
		{
			project->m_ProjectDirectory = path.parent_path();
			Projects::Project::s_ActiveProject = project;
			return Projects::Project::s_ActiveProject;
		}

		return nullptr;

	}
	bool AssetManager::SaveActiveProject(const std::filesystem::path& path)
	{
		if (Assets::AssetManager::SerializeProject(Projects::Project::s_ActiveProject, path))
		{
			Projects::Project::s_ActiveProject->m_ProjectDirectory = path.parent_path();
			return true;
		}
		return false;
	}

	bool AssetManager::SerializeProject(Ref<Projects::Project> project, const std::filesystem::path& filepath)
	{
		const auto& config = project->m_Config;
		YAML::Emitter out;

		{
			out << YAML::BeginMap; // Root
			out << YAML::Key << "Project" << YAML::Value;
			{
				out << YAML::BeginMap; // Project
				out << YAML::Key << "Name" << YAML::Value << config.Name;
				out << YAML::Key << "StartScene" << YAML::Value << config.StartScenePath.string();
				out << YAML::Key << "StartSceneHandle" << YAML::Value << static_cast<uint64_t>(config.StartSceneHandle);
				out << YAML::Key << "StartGameState" << YAML::Value << static_cast<uint64_t>(config.StartGameState);
				out << YAML::Key << "AssetDirectory" << YAML::Value << config.AssetDirectory.string();
				out << YAML::Key << "ScriptModulePath" << YAML::Value << config.ScriptModulePath.string();
				out << YAML::Key << "ScriptDLLPath" << YAML::Value << config.ScriptDLLPath.string();
				out << YAML::Key << "DefaultFullscreen" << YAML::Value << config.DefaultFullscreen;
				out << YAML::Key << "TargetResolution" << YAML::Value << Utility::ScreenResolutionToString(config.TargetResolution);
				out << YAML::Key << "OnRuntimeStart" << YAML::Value << static_cast<uint64_t>(config.OnRuntimeStart);
				out << YAML::Key << "OnUpdateUserCount" << YAML::Value << static_cast<uint64_t>(config.OnUpdateUserCount);
				out << YAML::Key << "OnApproveJoinSession" << YAML::Value << static_cast<uint64_t>(config.OnApproveJoinSession);
				out << YAML::Key << "OnUserLeftSession" << YAML::Value << static_cast<uint64_t>(config.OnUserLeftSession);
				out << YAML::Key << "OnCurrentSessionInit" << YAML::Value << static_cast<uint64_t>(config.OnCurrentSessionInit);
				out << YAML::Key << "OnConnectionTerminated" << YAML::Value << static_cast<uint64_t>(config.OnConnectionTerminated);
				out << YAML::Key << "OnUpdateSessionUserSlot" << YAML::Value << static_cast<uint64_t>(config.OnUpdateSessionUserSlot);
				out << YAML::Key << "OnStartSession" << YAML::Value << static_cast<uint64_t>(config.OnStartSession);
				out << YAML::Key << "OnSessionReadyCheckConfirmFunction" << YAML::Value << config.OnSessionReadyCheckConfirmFunction;
				out << YAML::Key << "OnReceiveSignalFunction" << YAML::Value << config.OnReceiveSignalFunction;
				out << YAML::Key << "AppIsNetworked" << YAML::Value << config.AppIsNetworked;

				if (config.AppTickGenerators.size() > 0)
				{
					out << YAML::Key << "AppTickGenerators" << YAML::BeginSeq;
					// Serialize App Tick Generators
					for (auto generatorValue : config.AppTickGenerators)
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

		return true;
	}

	bool AssetManager::DeserializeProject(Ref<Projects::Project> project, const std::filesystem::path& filepath)
	{
		auto& config = project->m_Config;

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

		config.Name = projectNode["Name"].as<std::string>();
		config.StartScenePath = projectNode["StartScene"].as<std::string>();
		config.StartSceneHandle = static_cast<AssetHandle>(projectNode["StartSceneHandle"].as<uint64_t>());
		config.StartGameState = static_cast<AssetHandle>(projectNode["StartGameState"].as<uint64_t>());
		config.AssetDirectory = projectNode["AssetDirectory"].as<std::string>();
		config.ScriptModulePath = projectNode["ScriptModulePath"].as<std::string>();
		config.ScriptDLLPath = projectNode["ScriptDLLPath"].as<std::string>();
		config.DefaultFullscreen = projectNode["DefaultFullscreen"].as<bool>();
		config.TargetResolution = Utility::StringToScreenResolution(projectNode["TargetResolution"].as<std::string>());
		config.OnRuntimeStart = static_cast<AssetHandle>(projectNode["OnRuntimeStart"].as<uint64_t>());
		config.OnUpdateUserCount = static_cast<AssetHandle>(projectNode["OnUpdateUserCount"].as<uint64_t>());
		config.OnApproveJoinSession = static_cast<AssetHandle>(projectNode["OnApproveJoinSession"].as<uint64_t>());
		config.OnUserLeftSession = static_cast<AssetHandle>(projectNode["OnUserLeftSession"].as<uint64_t>());
		config.OnCurrentSessionInit = static_cast<AssetHandle>(projectNode["OnCurrentSessionInit"].as<uint64_t>());
		config.OnConnectionTerminated = static_cast<AssetHandle>(projectNode["OnConnectionTerminated"].as<uint64_t>());
		config.OnUpdateSessionUserSlot = static_cast<AssetHandle>(projectNode["OnUpdateSessionUserSlot"].as<uint64_t>());
		config.OnStartSession = static_cast<AssetHandle>(projectNode["OnStartSession"].as<uint64_t>());
		config.OnSessionReadyCheckConfirmFunction = projectNode["OnSessionReadyCheckConfirmFunction"].as<std::string>();
		config.OnReceiveSignalFunction = projectNode["OnReceiveSignalFunction"].as<std::string>();
		config.AppIsNetworked = projectNode["AppIsNetworked"].as<bool>();

		auto tickGenerators = projectNode["AppTickGenerators"];

		if (tickGenerators)
		{
			for (auto generator : tickGenerators)
			{
				uint64_t value = generator.as<uint64_t>();
				config.AppTickGenerators.insert(value);
			}
		}

		DeserializeServerVariables(project, filepath);

		return true;
	}
}
