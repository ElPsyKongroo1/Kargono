#include "kgpch.h"

#include "Kargono/AssetsTemp/AssetService.h"
#include "Kargono/AssetsTemp/GameStateManagerTemp.h"

#include "Kargono/Scenes/GameState.h"

namespace Kargono::Assets
{

	bool GameStateManager::DeserializeGameState(Ref<Kargono::Scenes::GameState> GameState, const std::filesystem::path& filepath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load .kgui file '{0}'\n     {1}", filepath, e.what());
			return false;
		}

		KG_INFO("Deserializing game state");

		GameState->m_Name = data["Name"].as<std::string>();

		// Get Fields
		{
			auto fields = data["Fields"];
			if (fields)
			{
				auto& newFieldsMap = GameState->m_Fields;
				for (auto field : fields)
				{
					std::string fieldName = field["Name"].as<std::string>();
					WrappedVarType fieldType = Utility::StringToWrappedVarType(field["Type"].as<std::string>());
					Ref<WrappedVariable> wrappedVariable = Utility::DeserializeWrappedVariableData(fieldType, field);
					newFieldsMap.insert_or_assign(fieldName, wrappedVariable);
				}
			}
		}
		return true;

	}

	Ref<Scenes::GameState> GameStateManager::InstantiateAssetIntoMemory(Assets::Asset& asset)
	{
		Ref<Scenes::GameState> newGameState = CreateRef<Scenes::GameState>();
		DeserializeGameState(newGameState, (Projects::ProjectService::GetActiveAssetDirectory() / asset.Data.IntermediateLocation).string());
		return newGameState;
	}

	static GameStateManager s_GameStateManager;

	Ref<Scenes::GameState> AssetServiceTemp::GetGameState(const AssetHandle& handle)
	{
		return s_GameStateManager.GetAsset(handle);
	}

	std::filesystem::path Kargono::Assets::AssetServiceTemp::GetGameStateIntermediateLocation(const AssetHandle& handle)
	{
		return s_GameStateManager.GetAssetIntermediateLocation(handle);
	}
	
}
