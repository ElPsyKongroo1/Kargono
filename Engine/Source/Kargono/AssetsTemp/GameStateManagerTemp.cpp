#include "kgpch.h"

#include "Kargono/AssetsTemp/AssetService.h"
#include "Kargono/AssetsTemp/GameStateManagerTemp.h"

#include "Kargono/Scenes/GameState.h"

namespace Kargono::Assets
{
	Ref<Scenes::GameState> GameStateManager::DeserializeAsset(Assets::Asset& asset, const std::filesystem::path& assetPath)
	{
		Ref<Scenes::GameState> newGameState = CreateRef<Scenes::GameState>();
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(assetPath.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load .kgui file '{0}'\n     {1}", assetPath, e.what());
			return nullptr;
		}

		KG_INFO("Deserializing game state");

		newGameState->m_Name = data["Name"].as<std::string>();

		// Get Fields
		{
			auto fields = data["Fields"];
			if (fields)
			{
				auto& newFieldsMap = newGameState->m_Fields;
				for (auto field : fields)
				{
					std::string fieldName = field["Name"].as<std::string>();
					WrappedVarType fieldType = Utility::StringToWrappedVarType(field["Type"].as<std::string>());
					Ref<WrappedVariable> wrappedVariable = Utility::DeserializeWrappedVariableData(fieldType, field);
					newFieldsMap.insert_or_assign(fieldName, wrappedVariable);
				}
			}
		}

		return newGameState;
	}
	void GameStateManager::SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::Asset& currentAsset)
	{
		Assets::GameStateMetaData* metadata = currentAsset.Data.GetSpecificMetaData<GameStateMetaData>();
		serializer << YAML::Key << "Name" << YAML::Value << metadata->Name;
	}
	void GameStateManager::DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::Asset& currentAsset)
	{
		Ref<Assets::GameStateMetaData> GameStateMetaData = CreateRef<Assets::GameStateMetaData>();
		GameStateMetaData->Name = metadataNode["Name"].as<std::string>();
		currentAsset.Data.SpecificFileData = GameStateMetaData;
	}
}
