#include "kgpch.h"

#include "Kargono/Assets/AssetService.h"
#include "Kargono/Assets/AIStateManager.h"

#include "Kargono/AI/AIService.h"

namespace Kargono::Assets
{
	void AIStateManager::CreateAssetFileFromName(std::string_view name, AssetInfo& asset, const std::filesystem::path& assetPath)
	{
		UNREFERENCED_PARAMETER(name);
		UNREFERENCED_PARAMETER(asset);

		// Create Temporary AIState
		Ref<AI::AIState> temporaryAIState = CreateRef<AI::AIState>();

		// Save Binary into File
		SerializeAsset(temporaryAIState, assetPath);

		// Load data into In-Memory Metadata object
		Ref<Assets::AIStateMetaData> metadata = CreateRef<Assets::AIStateMetaData>();
		asset.Data.SpecificFileData = metadata;
	}

	void AIStateManager::SerializeAsset(Ref<AI::AIState> assetReference, const std::filesystem::path& assetPath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map

		out << YAML::Key << "OnUpdateScript" << YAML::Value << static_cast<uint64_t>(assetReference->OnUpdateHandle);
		out << YAML::Key << "OnEnterStateScript" << YAML::Value << static_cast<uint64_t>(assetReference->OnEnterStateHandle);
		out << YAML::Key << "OnExitStateScript" << YAML::Value << static_cast<uint64_t>(assetReference->OnExitStateHandle);
		out << YAML::Key << "OnAIMessageScript" << YAML::Value << static_cast<uint64_t>(assetReference->OnMessageHandle);

		out << YAML::EndMap; // End of File Map

		std::ofstream fout(assetPath);
		fout << out.c_str();
		KG_INFO("Successfully Serialized AIState at {}", assetPath.string());
	}
	Ref<AI::AIState> AIStateManager::DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath)
	{
		UNREFERENCED_PARAMETER(asset);
		Ref<AI::AIState> newAIState = CreateRef<AI::AIState>();
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(assetPath.string());
		}
		catch (YAML::ParserException e)
		{
			KG_WARN("Failed to load .kgui file '{0}'\n     {1}", assetPath, e.what());
			return nullptr;
		}

		// Deserialize OnUpdateScript
		newAIState->OnUpdateHandle = data["OnUpdateScript"].as<uint64_t>();
		if (newAIState->OnUpdateHandle == Assets::EmptyHandle)
		{
			newAIState->OnUpdate = nullptr;
		}
		else
		{
			newAIState->OnUpdate = Assets::AssetService::GetScript(newAIState->OnUpdateHandle);
			if (!newAIState->OnUpdate)
			{
				KG_WARN("Invalid OnUpdate script returned using a valid handle in deserialize ai state function");
			}
		}

		// Deserialize OnEnterStateScript
		newAIState->OnEnterStateHandle = data["OnEnterStateScript"].as<uint64_t>();
		if (newAIState->OnEnterStateHandle == Assets::EmptyHandle)
		{
			newAIState->OnEnterState = nullptr;
		}
		else
		{
			newAIState->OnEnterState = Assets::AssetService::GetScript(newAIState->OnEnterStateHandle);
			if (!newAIState->OnEnterState)
			{
				KG_WARN("Invalid OnEnterState script returned using a valid handle in deserialize ai state function");
			}
		}

		// Deserialize OnExitStateScript
		newAIState->OnExitStateHandle = data["OnExitStateScript"].as<uint64_t>();
		if (newAIState->OnExitStateHandle == Assets::EmptyHandle)
		{
			newAIState->OnExitState = nullptr;
		}
		else
		{
			newAIState->OnExitState = Assets::AssetService::GetScript(newAIState->OnExitStateHandle);
			if (!newAIState->OnExitState)
			{
				KG_WARN("Invalid OnExitState script returned using a valid handle in deserialize AI state function");
			}
		}

		// Deserialize OnAIMessageScript
		newAIState->OnMessageHandle = data["OnAIMessageScript"].as<uint64_t>();
		if (newAIState->OnMessageHandle == Assets::EmptyHandle)
		{
			newAIState->OnMessage = nullptr;
		}
		else
		{
			newAIState->OnMessage = Assets::AssetService::GetScript(newAIState->OnMessageHandle);
			if (!newAIState->OnMessage)
			{
				KG_WARN("Invalid AIMessage script returned using a valid handle in deserialize AI state function");
			}
		}

		return newAIState;
	}
	void AIStateManager::DeleteAssetValidation(AssetHandle assetHandle)
	{
		// Handle deleting the AI state by removing entity data from all scenes
		for (auto& [sceneHandle, assetInfo] : Assets::AssetService::GetSceneRegistry())
		{
			// Get scene
			Ref<Scenes::Scene> currentScene = Assets::AssetService::GetScene(sceneHandle);

			bool sceneModified = Assets::AssetService::RemoveAIStateFromScene(currentScene, assetHandle);

			if (sceneModified)
			{
				// Save scene asset on-disk 
				Assets::AssetService::SaveScene(sceneHandle, currentScene);
			}

		}
	}
	bool AIStateManager::RemoveScript(Ref<AI::AIState> aiStateRef, Assets::AssetHandle scriptHandle)
	{
		bool aiStateModified{ false };
		// Check AI State assets
		if (aiStateRef->OnEnterStateHandle == scriptHandle)
		{
			aiStateRef->OnEnterStateHandle = Assets::EmptyHandle;
			aiStateRef->OnEnterState = nullptr;
			aiStateModified = true;
		}

		if (aiStateRef->OnExitStateHandle == scriptHandle)
		{
			aiStateRef->OnExitStateHandle = Assets::EmptyHandle;
			aiStateRef->OnExitState = nullptr;
			aiStateModified = true;
		}

		if (aiStateRef->OnUpdateHandle == scriptHandle)
		{
			aiStateRef->OnUpdateHandle = Assets::EmptyHandle;
			aiStateRef->OnUpdate = nullptr;
			aiStateModified = true;
		}

		if (aiStateRef->OnMessageHandle == scriptHandle)
		{
			aiStateRef->OnMessageHandle = Assets::EmptyHandle;
			aiStateRef->OnMessage = nullptr;
			aiStateModified = true;
		}
		
		return aiStateModified;
	}
}
