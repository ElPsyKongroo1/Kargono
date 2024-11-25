#include "kgpch.h"

#include "Kargono/Assets/AssetService.h"
#include "Kargono/Assets/GameStateManager.h"

#include "Kargono/Scenes/GameState.h"

namespace Kargono::Assets
{
	void GameStateManager::CreateAssetFileFromName(const std::string& name, AssetInfo& asset, const std::filesystem::path& assetPath)
	{
		// Create Temporary GameState
		Ref<Scenes::GameState> temporaryGameState = CreateRef<Scenes::GameState>();
		temporaryGameState->SetName(name);

		SerializeAsset(temporaryGameState, assetPath);

		// Load data into In-Memory Metadata object
		Ref<Assets::GameStateMetaData> metadata = CreateRef<Assets::GameStateMetaData>();
		metadata->Name = name;
		asset.Data.SpecificFileData = metadata;
	}
	void GameStateManager::SerializeAsset(Ref<Scenes::GameState> assetReference, const std::filesystem::path& assetPath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map

		out << YAML::Key << "Name" << YAML::Value << assetReference->m_Name; // Output State Name

		out << YAML::Key << "Fields" << YAML::Value;
		out << YAML::BeginSeq; // Start Fields

		for (auto& [name, field] : assetReference->m_Fields)
		{
			out << YAML::BeginMap; // Start Field

			out << YAML::Key << "Name" << YAML::Value << name; // Name/Map Key
			out << YAML::Key << "Type" << YAML::Value << Utility::WrappedVarTypeToString(field->Type()); // Field Type
			Utility::SerializeWrappedVariableData(field, out); // Field Value

			out << YAML::EndMap; // End Field
		}

		out << YAML::EndSeq; // End Fields

		out << YAML::EndMap; // End of File Map

		std::ofstream fout(assetPath);
		fout << out.c_str();
		KG_INFO("Successfully Serialized GameState at {}", assetPath);
	}
	Ref<Scenes::GameState> GameStateManager::DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath)
	{
		Ref<Scenes::GameState> newGameState = CreateRef<Scenes::GameState>();
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
	void GameStateManager::SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::AssetInfo& currentAsset)
	{
		Assets::GameStateMetaData* metadata = currentAsset.Data.GetSpecificMetaData<GameStateMetaData>();
		serializer << YAML::Key << "Name" << YAML::Value << metadata->Name;
	}
	void GameStateManager::DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::AssetInfo& currentAsset)
	{
		Ref<Assets::GameStateMetaData> GameStateMetaData = CreateRef<Assets::GameStateMetaData>();
		GameStateMetaData->Name = metadataNode["Name"].as<std::string>();
		currentAsset.Data.SpecificFileData = GameStateMetaData;
	}
}
