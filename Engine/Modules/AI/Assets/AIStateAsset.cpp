#include "kgpch.h"

#include "Modules/AI/Assets/AIStateAsset.h"
#include "Kargono/Scenes/Scene.h"

#include "API/Serialization/yamlcppAPI.h"

namespace Kargono::AI
{
	void AIState::Deserialize(void* context)
	{
		KG_ASSERT(context, "Context cannot be null");

		// Get asset context
		Assets::DeserializeAssetContext& assetContext = *(Assets::DeserializeAssetContext*)context;

		// Get context fields
		KG_ASSERT(assetContext.m_AssetMetadata, "Metadata cannot be null");
		Assets::Metadata& metadata{ *assetContext.m_AssetMetadata };
		std::filesystem::path& assetPath{ assetContext.m_AssetPath };

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
		m_OnUpdateHandle = data["OnUpdateScript"].as<uint64_t>();
		if (m_OnUpdateHandle == Assets::k_EmptyHandle)
		{
			m_OnUpdate = nullptr;
		}
		else
		{
			m_OnUpdate = Assets::AssetService::GetScript(m_OnUpdateHandle);
			if (!m_OnUpdate)
			{
				KG_WARN("Invalid OnUpdate script returned using a valid handle in deserialize ai state function");
			}
		}

		// Deserialize OnEnterStateScript
		m_OnEnterStateHandle = data["OnEnterStateScript"].as<uint64_t>();
		if (m_OnEnterStateHandle == Assets::k_EmptyHandle)
		{
			m_OnEnterState = nullptr;
		}
		else
		{
			m_OnEnterState = Assets::AssetService::GetScript(m_OnEnterStateHandle);
			if (!m_OnEnterState)
			{
				KG_WARN("Invalid OnEnterState script returned using a valid handle in deserialize ai state function");
			}
		}

		// Deserialize OnExitStateScript
		m_OnExitStateHandle = data["OnExitStateScript"].as<uint64_t>();
		if (m_OnExitStateHandle == Assets::k_EmptyHandle)
		{
			m_OnExitState = nullptr;
		}
		else
		{
			m_OnExitState = Assets::AssetService::GetScript(m_OnExitStateHandle);
			if (!m_OnExitState)
			{
				KG_WARN("Invalid OnExitState script returned using a valid handle in deserialize AI state function");
			}
		}

		// Deserialize OnAIMessageScript
		m_OnMessageHandle = data["OnAIMessageScript"].as<uint64_t>();
		if (m_OnMessageHandle == Assets::k_EmptyHandle)
		{
			m_OnMessage = nullptr;
		}
		else
		{
			m_OnMessage = Assets::AssetService::GetScript(m_OnMessageHandle);
			if (!m_OnMessage)
			{
				KG_WARN("Invalid AIMessage script returned using a valid handle in deserialize AI state function");
			}
		}
	}

	void AIState::CreateAssetFileFromName(std::string_view name, Assets::Metadata& metadata, std::filesystem::path& path)
	{
		// Create Temporary AIState
		AIState temporaryAIState{};

		// Save Binary into File
		Assets::SerializeAssetContext serializeContext{};
		serializeContext.m_AssetPath = path;
		temporaryAIState.Serialize((void*)&serializeContext);
	}

	void AIState::DeleteValidation(Assets::Metadata& metadata)
	{
		// Handle deleting the AI state by removing entity data from all scenes
		for (auto& [sceneHandle, assetInfo] : Assets::AssetService::GetSceneRegistry())
		{
			// Get scene
			Ref<Scenes::Scene> currentScene = Assets::AssetService::GetScene(sceneHandle);

			bool sceneModified = Assets::AssetService::RemoveAIStateFromScene(currentScene, metadata.m_Handle);

			if (sceneModified)
			{
				// Save scene asset on-disk 
				Assets::AssetService::SaveScene(sceneHandle, currentScene);
			}
		}
	}

	void AIState::Serialize(void* context)
	{
		// Get asset context
		Assets::SerializeAssetContext& assetContext = *(Assets::SerializeAssetContext*)context;

		// Get context fields
		std::filesystem::path& assetPath{ assetContext.m_AssetPath };

		// Serialize
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map
		out << YAML::Key << "OnUpdateScript" << YAML::Value << static_cast<uint64_t>(m_OnUpdateHandle);
		out << YAML::Key << "OnEnterStateScript" << YAML::Value << static_cast<uint64_t>(m_OnEnterStateHandle);
		out << YAML::Key << "OnExitStateScript" << YAML::Value << static_cast<uint64_t>(m_OnExitStateHandle);
		out << YAML::Key << "OnAIMessageScript" << YAML::Value << static_cast<uint64_t>(m_OnMessageHandle);
		out << YAML::EndMap; // End of File Map

		// Save into file
		std::ofstream fout(assetPath);
		fout << out.c_str();
		KG_INFO("Successfully Serialized AIState at {}", assetPath.string());
	}

	bool AIState::RemoveScript(Assets::AssetHandle scriptHandle)
	{
		bool aiStateModified{ false };
		// Check AI State assets
		if (m_OnEnterStateHandle == scriptHandle)
		{
			m_OnEnterStateHandle = Assets::k_EmptyHandle;
			m_OnEnterState = nullptr;
			aiStateModified = true;
		}

		if (m_OnExitStateHandle == scriptHandle)
		{
			m_OnExitStateHandle = Assets::k_EmptyHandle;
			m_OnExitState = nullptr;
			aiStateModified = true;
		}

		if (m_OnUpdateHandle == scriptHandle)
		{
			m_OnUpdateHandle = Assets::k_EmptyHandle;
			m_OnUpdate = nullptr;
			aiStateModified = true;
		}

		if (m_OnMessageHandle == scriptHandle)
		{
			m_OnMessageHandle = Assets::k_EmptyHandle;
			m_OnMessage = nullptr;
			aiStateModified = true;
		}

		return aiStateModified;
	}
}