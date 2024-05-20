#include "kgpch.h"

#include "Kargono/Assets/AssetManager.h"
#include "Kargono/Projects/Project.h"
#include "API/Serialization/SerializationAPI.h"
#include "Kargono/Utility/FileSystem.h"


namespace Kargono::Utility
{
	void SerializeWrappedVariableData(Ref<WrappedVariable> variable, YAML::Emitter& out)
	{
		switch (variable->Type())
		{
		case WrappedVarType::Integer32:
		{
			out << YAML::Key << "Value" << YAML::Value << variable->GetWrappedValue<int32_t>();
			return;
		}
		case WrappedVarType::UInteger16:
		{
			out << YAML::Key << "Value" << YAML::Value << variable->GetWrappedValue<uint16_t>();
			return;
		}
		case WrappedVarType::UInteger32:
		{
			out << YAML::Key << "Value" << YAML::Value << variable->GetWrappedValue<uint32_t>();
			return;
		}
		case WrappedVarType::UInteger64:
		{
			out << YAML::Key << "Value" << YAML::Value << variable->GetWrappedValue<int64_t>();
			return;
		}
		case WrappedVarType::String:
		{
			out << YAML::Key << "Value" << YAML::Value << variable->GetWrappedValue<std::string>();
			return;
		}
		case WrappedVarType::Bool:
		{
			out << YAML::Key << "Value" << YAML::Value << variable->GetWrappedValue<bool>();
			return;
		}
		case WrappedVarType::Void:
		case WrappedVarType::None:
		{
			KG_CRITICAL("Use of Void or None when serializing value");
			return;
		}
		}
		KG_ASSERT(false, "Unknown Type of WrappedVariableType when serializing");
	}

	Ref<WrappedVariable> DeserializeWrappedVariableData(WrappedVarType type, YAML::detail::iterator_value& field)
	{
		switch (type)
		{
		case WrappedVarType::Integer32:
		{
			return CreateRef<WrappedInteger32>(field["Value"].as<int32_t>());
		}
		case WrappedVarType::UInteger16:
		{
			return CreateRef<WrappedUInteger16>(field["Value"].as<uint16_t>());
		}
		case WrappedVarType::UInteger32:
		{
			return CreateRef<WrappedUInteger32>(field["Value"].as<uint32_t>());
		}
		case WrappedVarType::UInteger64:
		{
			return CreateRef<WrappedUInteger64>(field["Value"].as<uint64_t>());
		}
		case WrappedVarType::String:
		{
			return CreateRef<WrappedString>(field["Value"].as<std::string>());
		}
		case WrappedVarType::Bool:
		{
			return CreateRef<WrappedBool>(field["Value"].as<bool>());
		}
		case WrappedVarType::Void:
		case WrappedVarType::None:
		{
			KG_CRITICAL("Use of Void or None when serializing value");
			return nullptr;
		}
		}
		KG_ASSERT(false, "Unknown Type of WrappedVariableType when deserializing");
		return nullptr;
	}
}

namespace Kargono::Assets
{
	std::unordered_map<AssetHandle, Assets::Asset> AssetManager::s_GameStateRegistry {};

	void AssetManager::DeserializeGameStateRegistry()
	{
		// Clear current registry and open registry in current project 
		s_GameStateRegistry.clear();
		KG_ASSERT(Projects::Project::GetActive(), "There is no currently loaded project to serialize from!");
		const auto& GameStateRegistryLocation = Projects::Project::GetAssetDirectory() / "GameState/GameStateRegistry.kgreg";

		if (!std::filesystem::exists(GameStateRegistryLocation))
		{
			KG_ERROR("No .kgregistry file exists in project path!");
			return;
		}
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(GameStateRegistryLocation.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load .kgstate file '{0}'\n     {1}", GameStateRegistryLocation.string(), e.what());
			return;
		}

		// Opening registry node 
		if (!data["Registry"]) { return; }

		std::string registryName = data["Registry"].as<std::string>();
		KG_INFO("Deserializing GameState Registry");

		// Opening all assets 
		auto assets = data["Assets"];
		if (assets)
		{
			for (auto asset : assets)
			{
				Assets::Asset newAsset{};
				newAsset.Handle = asset["AssetHandle"].as<uint64_t>();

				// Retrieving metadata for asset 
				auto metadata = asset["MetaData"];
				newAsset.Data.CheckSum = metadata["CheckSum"].as<std::string>();
				newAsset.Data.IntermediateLocation = metadata["IntermediateLocation"].as<std::string>();
				newAsset.Data.Type = Utility::StringToAssetType(metadata["AssetType"].as<std::string>());

				// Retrieving GameState specific metadata 
				if (newAsset.Data.Type == Assets::GameState)
				{
					Ref<Assets::GameStateMetaData> GameStateMetaData = CreateRef<Assets::GameStateMetaData>();
					GameStateMetaData->Name = metadata["Name"].as<std::string>();
					newAsset.Data.SpecificFileData = GameStateMetaData;
				}

				// Add asset to in memory registry 
				s_GameStateRegistry.insert({ newAsset.Handle, newAsset });
			}
		}
	}

	void AssetManager::SerializeGameStateRegistry()
	{
		KG_ASSERT(Projects::Project::GetActive(), "There is no currently loaded project to serialize to!");
		const auto& GameStateRegistryLocation = Projects::Project::GetAssetDirectory() / "GameState/GameStateRegistry.kgreg";
		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "Registry" << YAML::Value << "GameState";
		out << YAML::Key << "Assets" << YAML::Value << YAML::BeginSeq;

		// Asset
		for (auto& [handle, asset] : s_GameStateRegistry)
		{
			out << YAML::BeginMap; // Asset Map
			out << YAML::Key << "AssetHandle" << YAML::Value << static_cast<uint64_t>(handle);
			out << YAML::Key << "MetaData" << YAML::Value;
			out << YAML::BeginMap; // MetaData Map
			out << YAML::Key << "CheckSum" << YAML::Value << asset.Data.CheckSum;
			out << YAML::Key << "IntermediateLocation" << YAML::Value << asset.Data.IntermediateLocation.string();
			out << YAML::Key << "AssetType" << YAML::Value << Utility::AssetTypeToString(asset.Data.Type);
			if (asset.Data.Type == Assets::AssetType::GameState)
			{
				Assets::GameStateMetaData* metadata = static_cast<Assets::GameStateMetaData*>(asset.Data.SpecificFileData.get());

				out << YAML::Key << "Name" << YAML::Value << metadata->Name;
			}

			out << YAML::EndMap; // MetaData Map
			out << YAML::EndMap; // Asset Map
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		Utility::FileSystem::CreateNewDirectory(GameStateRegistryLocation.parent_path());

		std::ofstream fout(GameStateRegistryLocation);
		fout << out.c_str();
	}

	void AssetManager::SerializeGameState(Ref<Kargono::GameState> GameState, const std::filesystem::path& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map

		out << YAML::Key << "Name" << YAML::Value << GameState->m_Name; // Output State Name

		out << YAML::Key << "Fields" << YAML::Value;
		out << YAML::BeginSeq; // Start Fields

		for (auto& [name, field] : GameState->m_Fields)
		{
			out << YAML::BeginMap; // Start Field

			out << YAML::Key << "Name" << YAML::Value << name; // Name/Map Key
			out << YAML::Key << "Type" << YAML::Value << Utility::WrappedVarTypeToString(field->Type()); // Field Type
			Utility::SerializeWrappedVariableData(field, out); // Field Value

			out << YAML::EndMap; // End Field
		}

		out << YAML::EndSeq; // End Fields

		out << YAML::EndMap; // End of File Map

		std::ofstream fout(filepath);
		fout << out.c_str();
		KG_INFO("Successfully Serialized GameState at {}", filepath);
	}

	bool AssetManager::CheckGameStateExists(const std::string& GameStateName)
	{
		// Create Checksum
		const std::string currentCheckSum = Utility::FileSystem::ChecksumFromString(GameStateName);

		if (currentCheckSum.empty())
		{
			KG_ERROR("Failed to generate checksum from file!");
			return {};
		}

		for (const auto& [handle, asset] : s_GameStateRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_INFO("Attempt to instantiate duplicate font asset");
				return true;
			}
		}

		return false;
	}

	bool AssetManager::DeserializeGameState(Ref<Kargono::GameState> GameState, const std::filesystem::path& filepath)
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

	AssetHandle AssetManager::CreateNewGameState(const std::string& GameStateName)
	{
		// Create Checksum
		const std::string currentCheckSum = Utility::FileSystem::ChecksumFromString(GameStateName);

		if (currentCheckSum.empty())
		{
			KG_ERROR("Failed to generate checksum from file!");
			return {};
		}

		// Compare currentChecksum to registered assets
		for (const auto& [handle, asset] : s_GameStateRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_INFO("Attempt to instantiate duplicate game state asset");
				return handle;
			}
		}

		// Create New Asset/Handle
		AssetHandle newHandle{};
		Assets::Asset newAsset{};
		newAsset.Handle = newHandle;

		// Create File
		CreateGameStateFile(GameStateName, newAsset);
		newAsset.Data.CheckSum = currentCheckSum;

		// Register New Asset and return handle.
		s_GameStateRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeGameStateRegistry(); // Update Registry File on Disk

		return newHandle;
	}

	void AssetManager::SaveGameState(AssetHandle GameStateHandle, Ref<Kargono::GameState> GameState)
	{
		if (!s_GameStateRegistry.contains(GameStateHandle))
		{
			KG_ERROR("Attempt to save GameState that does not exist in registry");
			return;
		}
		Assets::Asset GameStateAsset = s_GameStateRegistry[GameStateHandle];
		SerializeGameState(GameState, (Projects::Project::GetAssetDirectory() / GameStateAsset.Data.IntermediateLocation).string());
	}

	void AssetManager::DeleteGameState(AssetHandle handle)
	{
		if (!s_GameStateRegistry.contains(handle))
		{
			KG_WARN("Failed to delete GameState in AssetManager");
			return;
		}

		Utility::FileSystem::DeleteSelectedFile(Projects::Project::GetAssetDirectory() /
			s_GameStateRegistry.at(handle).Data.IntermediateLocation);

		s_GameStateRegistry.erase(handle);

		SerializeGameStateRegistry();
	}

	std::filesystem::path AssetManager::GetGameStateLocation(const AssetHandle& handle)
	{
		if (!s_GameStateRegistry.contains(handle))
		{
			KG_ERROR("Attempt to save GameState that does not exist in registry");
			return "";
		}
		return s_GameStateRegistry[handle].Data.IntermediateLocation;
	}

	Ref<Kargono::GameState> AssetManager::GetGameState(const AssetHandle& handle)
	{
		KG_ASSERT(Projects::Project::GetActive(), "There is no active project when retreiving GameState!");

		if (s_GameStateRegistry.contains(handle))
		{
			auto asset = s_GameStateRegistry[handle];
			return InstantiateGameState(asset);
		}

		KG_ERROR("No GameState is associated with provided handle!");
		return nullptr;
	}
	std::tuple<AssetHandle, Ref<Kargono::GameState>> AssetManager::GetGameState(const std::filesystem::path& filepath)
	{
		KG_ASSERT(Projects::Project::GetActive(), "Attempt to use Project Field without active project!");

		std::filesystem::path GameStatePath = filepath;

		if (filepath.is_absolute())
		{
			GameStatePath = Utility::FileSystem::GetRelativePath(Projects::Project::GetAssetDirectory(), filepath);
		}

		for (auto& [assetHandle, asset] : s_GameStateRegistry)
		{
			if (asset.Data.IntermediateLocation.compare(GameStatePath) == 0)
			{
				return std::make_tuple(assetHandle, InstantiateGameState(asset));
			}
		}
		// Return empty GameState if GameState does not exist
		KG_WARN("No GameState Associated with provided handle. Returned new empty GameState");
		AssetHandle newHandle = CreateNewGameState(filepath.stem().string());
		return std::make_tuple(newHandle, GetGameState(newHandle));
	}

	Ref<Kargono::GameState> AssetManager::InstantiateGameState(const Assets::Asset& GameStateAsset)
	{
		Ref<Kargono::GameState> newGameState = CreateRef<Kargono::GameState>();
		DeserializeGameState(newGameState, (Projects::Project::GetAssetDirectory() / GameStateAsset.Data.IntermediateLocation).string());
		return newGameState;
	}


	void AssetManager::ClearGameStateRegistry()
	{
		s_GameStateRegistry.clear();
	}

	void AssetManager::CreateGameStateFile(const std::string& GameStateName, Assets::Asset& newAsset)
	{
		// Create Temporary GameState
		Ref<Kargono::GameState> temporaryGameState = CreateRef<Kargono::GameState>();
		temporaryGameState->SetName(GameStateName);

		// Save Binary Intermediate into File
		std::string GameStatePath = "GameState/" + GameStateName + ".kgstate";
		std::filesystem::path intermediateFullPath = Projects::Project::GetAssetDirectory() / GameStatePath;
		SerializeGameState(temporaryGameState, intermediateFullPath.string());

		// Load data into In-Memory Metadata object
		newAsset.Data.Type = Assets::AssetType::GameState;
		newAsset.Data.IntermediateLocation = GameStatePath;
		Ref<Assets::GameStateMetaData> metadata = CreateRef<Assets::GameStateMetaData>();
		metadata->Name = GameStateName;
		newAsset.Data.SpecificFileData = metadata;
	}
}
