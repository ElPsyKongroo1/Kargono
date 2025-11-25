#include "kgpch.h"

#include "Modules/GlobalState/Assets/GameState.h"

namespace Kargono::GlobalState
{
	void GameState::CreateFromName(Assets::Metadata<GameState>& metadata)
	{
		// Create default game state
		GameState defaultGameState{};
		defaultGameState.SetName(metadata.m_Name.StringView());

		// Serialize default game state into asset
		Assets::SerializeAssetContext<GameState> assetContext{};
		assetContext.m_AssetMetadata = &metadata;
		defaultGameState.Serialize((void*)&assetContext);
	}
	void GameState::Serialize(void* context)
	{
		// Get context
		Assets::SerializeAssetContext<GameState>* serializeContext = (Assets::SerializeAssetContext<GameState>*)context;
		KG_ASSERT(serializeContext);
		Assets::Metadata<GameState>* metadata{ serializeContext->m_AssetMetadata };
		KG_ASSERT(metadata);

		// Get asset path
		const std::filesystem::path& assetPath{ metadata->GetAssetFullFilePath() };

		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map

		out << YAML::Key << "Name" << YAML::Value << m_Name; // Output State Name

		out << YAML::Key << "Fields" << YAML::Value;
		out << YAML::BeginSeq; // Start Fields

		for (auto& [name, field] : m_Fields)
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
	void GameState::Deserialize(void* context)
	{
		// Get asset context
		Assets::DeserializeAssetContext<GameState>* assetContext = (Assets::DeserializeAssetContext<GameState>*)context;
		KG_ASSERT(assetContext, "Context cannot be null");
		Assets::Metadata<GameState>* metadata{ assetContext->m_AssetMetadata };
		KG_ASSERT(metadata, "Metadata cannot be null");

		// Get context fields
		const std::filesystem::path& assetPath{ metadata->GetAssetFullFilePath() };

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
		m_Name = data["Name"].as<std::string>();

		// Get Fields
		YAML::Node fields = data["Fields"];
		if (fields)
		{
			for (YAML::iterator::value_type field : fields)
			{
				std::string fieldName = field["Name"].as<std::string>();
				WrappedVarType fieldType = Utility::StringToWrappedVarType(field["Type"].as<std::string>());
				Ref<WrappedVariable> wrappedVariable = Utility::DeserializeWrappedVariableData(fieldType, field);
				m_Fields.insert_or_assign(fieldName, wrappedVariable);
			}
		}
	}

	Ref<WrappedVariable> GameState::GetField(std::string_view fieldName)
	{
		// TODO: This is horrendous. We shouldn't be using this in production anyways, but still... ughh
		std::string fieldNameString{ fieldName };
		if (!m_Fields.contains(fieldNameString))
		{
			KG_CRITICAL("Could not get field from game state {}", fieldName);
			return nullptr;
		}
		return m_Fields.at(fieldNameString);
	}

	void GameState::SetField(std::string_view fieldName, void* value)
	{
		// TODO: This is horrendous. We shouldn't be using this in production anyways, but still... ughh
		std::string fieldNameString{ fieldName };
		if (!m_Fields.contains(fieldNameString))
		{
			KG_CRITICAL("Could not get field from game state {}", fieldName);
			return;
		}

		m_Fields.at(fieldNameString)->SetValue(value);
	}

	bool GameState::AddField(const std::string& fieldName, WrappedVarType fieldType)
	{
		if (m_Fields.contains(fieldName))
		{
			KG_WARN("Attempt to add field to Game State that already exists");
			return false;
		}
		Ref<WrappedVariable> newVariable = nullptr;

		switch (fieldType)
		{
		case WrappedVarType::UInteger16:
		{
			newVariable = CreateRef<WrappedUInteger16>();
			m_Fields.insert_or_assign(fieldName, newVariable);
			return true;
		}
		default:
		{
			KG_WARN("Could not add field to Game State. Invalid fieldType Given.");
			return false;
		}
		}
	}

	bool GameState::DeleteField(const std::string& fieldName)
	{
		if (!m_Fields.contains(fieldName))
		{
			KG_WARN("Attempt to delete field to Game State that does not exist");
			return false;
		}

		m_Fields.erase(fieldName);
		return true;
	}
}