#include "kgpch.h"

#include "Modules/States/Assets/State.h"
#include "Modules/Scenes/Assets/Scene.h"
#include "Modules/Assets/Managers/ScriptManager.h"
#include "Modules/Assets/Managers/SceneManager.h"

#include "API/Serialization/yamlcppAPI.h"

namespace Kargono::States
{
	void State::Deserialize(void* context)
	{
		KG_ASSERT(context, "Context cannot be null");

		// Get asset context
		Assets::DeserializeAssetContext<State>& assetContext = *(Assets::DeserializeAssetContext<State>*)context;

		// Get context fields
		KG_ASSERT(assetContext.m_AssetMetadata, "Metadata cannot be null");
		Assets::Metadata<State>* metadata{ assetContext.m_AssetMetadata };
		KG_ASSERT(metadata);
		const std::filesystem::path& assetPath = metadata->GetAssetFullFilePath();

		YAML::Node data;
		try
		{
			data = YAML::LoadFile(assetPath.string());
		}
		catch (YAML::ParserException e)
		{
			KG_WARN("Failed to load .kgui file '{0}'\n     {1}", assetPath, e.what());
			return;
		}

		// Deserialize OnUpdateScript
		Assets::AssetHandle onUpdateHandle = data["OnUpdateScript"].as<uint64_t>();
		if (!onUpdateHandle.IsValid())
		{
			m_OnUpdate.Reset();
		}
		else
		{
			m_OnUpdate = Assets::s_ScriptManager.GetAssetByHandle(onUpdateHandle);
			if (!m_OnUpdate)
			{
				KG_WARN("Invalid OnUpdate script returned using a valid handle in deserialize ai state function");
			}
		}

		// Deserialize OnEnterStateScript
		Assets::AssetHandle onEnterStateHandle = data["OnEnterStateScript"].as<uint64_t>();
		if (!onEnterStateHandle.IsValid())
		{
			m_OnEnterState.Reset();
		}
		else
		{
			m_OnEnterState = Assets::s_ScriptManager.GetAssetByHandle(onEnterStateHandle);
			if (!m_OnEnterState)
			{
				KG_WARN("Invalid OnEnterState script returned using a valid handle in deserialize ai state function");
			}
		}

		// Deserialize OnExitStateScript
		Assets::AssetHandle onExitStateHandle = data["OnExitStateScript"].as<uint64_t>();
		if (!onExitStateHandle.IsValid())
		{
			m_OnExitState.Reset();
		}
		else
		{
			m_OnExitState = Assets::s_ScriptManager.GetAssetByHandle(onExitStateHandle);
			if (!m_OnExitState)
			{
				KG_WARN("Invalid OnExitState script returned using a valid handle in deserialize AI state function");
			}
		}

		// Deserialize OnAIMessageScript
		Assets::AssetHandle onMessageHandle = data["OnAIMessageScript"].as<uint64_t>();
		if (!onMessageHandle.IsValid())
		{
			m_OnMessage.Reset();
		}
		else
		{
			m_OnMessage = Assets::s_ScriptManager.GetAssetByHandle(onMessageHandle);
			if (!m_OnMessage)
			{
				KG_WARN("Invalid AIMessage script returned using a valid handle in deserialize AI state function");
			}
		}
	}

	void State::Serialize(void* context)
	{
		// Get asset context
		KG_ASSERT(context);
		Assets::SerializeAssetContext<State>& assetContext = *(Assets::SerializeAssetContext<State>*)context;

		// Get context fields
		Assets::Metadata<State>* metadata = assetContext.m_AssetMetadata;
		KG_ASSERT(metadata);

		const std::filesystem::path& assetPath{ metadata->GetAssetFullFilePath() };

		// Serialize
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map
		out << YAML::Key << "OnUpdateScript" << YAML::Value << static_cast<uint64_t>(m_OnUpdate.GetAssetHandle());
		out << YAML::Key << "OnEnterStateScript" << YAML::Value << static_cast<uint64_t>(m_OnEnterState.GetAssetHandle());
		out << YAML::Key << "OnExitStateScript" << YAML::Value << static_cast<uint64_t>(m_OnExitState.GetAssetHandle());
		out << YAML::Key << "OnAIMessageScript" << YAML::Value << static_cast<uint64_t>(m_OnMessage.GetAssetHandle());
		out << YAML::EndMap; // End of File Map

		// Save into file
		std::ofstream fout(assetPath);
		fout << out.c_str();
		KG_INFO("Successfully Serialized State at {}", assetPath.string());
	}

	void State::CreateFromName(Assets::Metadata<State>& metadata)
	{
		// Create default StateMachines State
		State temporaryState{};

		// Serialize a default copy
		Assets::SerializeAssetContext<State> serializeContext{};
		serializeContext.m_AssetMetadata = &metadata;
		temporaryState.Serialize((void*)&serializeContext);
	}

	void State::ValidateDelete(Assets::Metadata<State>& metadata)
	{
		// Handle deleting the StateMachines state by removing entity data from all scenes
		for (auto& [sceneHandle, metadata] : Assets::s_SceneManager.GetAssetRegistry())
		{
			// Get scene
			Assets::AssetRef<Scenes::Scene> currentScene = Assets::s_SceneManager.GetAssetByHandle(sceneHandle);

			currentScene->RemoveState(metadata.m_Handle);

			bool sceneModified = currentScene->RemoveState(metadata.m_Handle);

			if (sceneModified)
			{
				// Save scene asset on-disk 
				Assets::s_SceneManager.UpdateAsset(currentScene);
			}
		}
	}

	bool State::RemoveScript(Assets::AssetHandle scriptHandle)
	{
		bool aiStateModified{ false };
		// Check state assets
		if (m_OnEnterState.GetAssetHandle() == scriptHandle)
		{
			m_OnEnterState.Reset();
			aiStateModified = true;
		}

		if (m_OnExitState.GetAssetHandle() == scriptHandle)
		{
			m_OnExitState.Reset();
			aiStateModified = true;
		}

		if (m_OnUpdate.GetAssetHandle() == scriptHandle)
		{
			m_OnUpdate.Reset();
			aiStateModified = true;
		}

		if (m_OnMessage.GetAssetHandle() == scriptHandle)
		{
			m_OnMessage.Reset();
			aiStateModified = true;
		}

		return aiStateModified;
	}
}