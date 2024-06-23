#include "kgpch.h"

#include "Kargono/Assets/AssetManager.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Utility/FileSystem.h"

#include "API/Serialization/yamlcppAPI.h"


namespace Kargono::Assets
{
	std::unordered_map<AssetHandle, Assets::Asset> AssetManager::s_InputModeRegistry {};

	void AssetManager::DeserializeInputModeRegistry()
	{
		// Clear current registry and open registry in current project 
		s_InputModeRegistry.clear();
		KG_ASSERT(Projects::Project::GetActive(), "There is no currently loaded project to serialize from!");
		const auto& inputModeRegistryLocation = Projects::Project::GetAssetDirectory() / "Input/InputRegistry.kgreg";

		if (!std::filesystem::exists(inputModeRegistryLocation))
		{
			KG_ERROR("No .kgregistry file exists in project path!");
			return;
		}
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(inputModeRegistryLocation.string());
		}
		catch (YAML::ParserException e)
		{
			KG_ERROR("Failed to load .kginputMode file '{0}'\n     {1}", inputModeRegistryLocation.string(), e.what());
			return;
		}

		// Opening registry node 
		if (!data["Registry"]) { return; }

		std::string registryName = data["Registry"].as<std::string>();
		KG_INFO("Deserializing InputMode Registry");

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

				// Retrieving inputMode specific metadata 
				if (newAsset.Data.Type == Assets::InputMode)
				{
					Ref<Assets::InputModeMetaData> inputModeMetaData = CreateRef<Assets::InputModeMetaData>();
					newAsset.Data.SpecificFileData = inputModeMetaData;
				}

				// Add asset to in memory registry 
				s_InputModeRegistry.insert({ newAsset.Handle, newAsset });
			}
		}
	}

	void AssetManager::SerializeInputModeRegistry()
	{
		KG_ASSERT(Projects::Project::GetActive(), "There is no currently loaded project to serialize to!");
		const auto& inputModeRegistryLocation = Projects::Project::GetAssetDirectory() / "Input/InputRegistry.kgreg";
		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "Registry" << YAML::Value << "UserInterface";
		out << YAML::Key << "Assets" << YAML::Value << YAML::BeginSeq;

		// Asset
		for (auto& [handle, asset] : s_InputModeRegistry)
		{
			out << YAML::BeginMap; // Asset Map
			out << YAML::Key << "AssetHandle" << YAML::Value << static_cast<uint64_t>(handle);
			out << YAML::Key << "MetaData" << YAML::Value;
			out << YAML::BeginMap; // MetaData Map
			out << YAML::Key << "CheckSum" << YAML::Value << asset.Data.CheckSum;
			out << YAML::Key << "IntermediateLocation" << YAML::Value << asset.Data.IntermediateLocation.string();
			out << YAML::Key << "AssetType" << YAML::Value << Utility::AssetTypeToString(asset.Data.Type);

			out << YAML::EndMap; // MetaData Map
			out << YAML::EndMap; // Asset Map
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		Utility::FileSystem::CreateNewDirectory(inputModeRegistryLocation.parent_path());

		std::ofstream fout(inputModeRegistryLocation);
		fout << out.c_str();
	}

	void AssetManager::SerializeInputMode(Ref<Input::InputMode> inputMode, const std::filesystem::path& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map

		{
			// Keyboard Polling
			out << YAML::Key << "KeyboardPolling" << YAML::Value;
			out << YAML::BeginSeq; // Start of KeyboardPolling Seq
			uint32_t iteration{ 0 };
			for (auto keyCode : inputMode->m_KeyboardPolling)
			{
				out << YAML::BeginMap; // Start Polling Combo

				out << YAML::Key << "Slot" << YAML::Value << iteration;
				out << YAML::Key << "KeyCode" << YAML::Value << keyCode;

				out << YAML::EndMap; // End Polling Combo
				iteration++;
			}
			out << YAML::EndSeq; // End of KeyboardPolling Seq
		}

		{
			// OnUpdate
			out << YAML::Key << "OnUpdate" << YAML::Value;
			out << YAML::BeginSeq; // Start of OnUpdate Seq

			for (auto& inputBinding : inputMode->m_OnUpdateBindings)
			{
				out << YAML::BeginMap; // InputActionBinding Start

				out << YAML::Key << "BindingType" << YAML::Value << Utility::InputActionTypeToString(inputBinding->GetActionType());
				out << YAML::Key << "ScriptHandle" << YAML::Value << static_cast<uint64_t>(inputBinding->GetScriptHandle());

				switch (inputBinding->GetActionType())
				{
				case Input::InputActionTypes::KeyboardAction:
				{
					Input::KeyboardActionBinding* keyboardBinding = (Input::KeyboardActionBinding*)inputBinding.get();
					out << YAML::Key << "KeyBinding" << YAML::Value << keyboardBinding->GetKeyBinding();
					break;
				}
				case Input::InputActionTypes::None:
				default:
				{
					KG_ERROR("Invalid InputMode provided to InputMode serialization");
					break;
				}
				}

				out << YAML::EndMap; // InputActionBinding End
			}
			out << YAML::EndSeq; // End of OnUpdate Seq
		}

		{
			// OnKeyPressed
			out << YAML::Key << "OnKeyPressed" << YAML::Value;
			out << YAML::BeginSeq; // Start of OnKeyPressed Seq

			for (auto& inputBinding : inputMode->m_OnKeyPressedBindings)
			{
				out << YAML::BeginMap; // InputActionBinding Start

				out << YAML::Key << "BindingType" << YAML::Value << Utility::InputActionTypeToString(inputBinding->GetActionType());
				out << YAML::Key << "ScriptHandle" << YAML::Value << static_cast<uint64_t>(inputBinding->GetScriptHandle());

				switch (inputBinding->GetActionType())
				{
				case Input::InputActionTypes::KeyboardAction:
				{
					Input::KeyboardActionBinding* keyboardBinding = (Input::KeyboardActionBinding*)inputBinding.get();
					out << YAML::Key << "KeyBinding" << YAML::Value << keyboardBinding->GetKeyBinding();
					break;
				}
				case Input::InputActionTypes::None:
				default:
				{
					KG_ASSERT("Invalid InputMode provided to InputMode serialization");
					break;
				}
				}

				out << YAML::EndMap; // InputActionBinding End
			}
			out << YAML::EndSeq; // End of OnKeyPressed Seq
		}

		out << YAML::EndMap; // Start of File Map

		std::ofstream fout(filepath);
		fout << out.c_str();
		KG_INFO("Successfully Serialized InputMode at {}", filepath);
	}

	bool AssetManager::CheckInputModeExists(const std::string& inputModeName)
	{
		// Create Checksum
		const std::string currentCheckSum = Utility::FileSystem::ChecksumFromString(inputModeName);

		if (currentCheckSum.empty())
		{
			KG_ERROR("Failed to generate checksum from file!");
			return {};
		}

		for (const auto& [handle, asset] : s_InputModeRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_INFO("Attempt to instantiate duplicate font asset");
				return true;
			}
		}

		return false;
	}

	bool AssetManager::DeserializeInputMode(Ref<Input::InputMode> inputMode, const std::filesystem::path& filepath)
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

		KG_INFO("Deserializing input mode");

		// Get Keyboard Polling!
		{
			auto keyboardPolling = data["KeyboardPolling"];
			if (keyboardPolling)
			{
				auto& keyboardPollingNew = inputMode->m_KeyboardPolling;
				for (auto binding : keyboardPolling)
				{
					uint16_t slot = (uint16_t)binding["Slot"].as<uint32_t>();
					uint16_t keyCode = (uint16_t)binding["KeyCode"].as<uint32_t>();
					keyboardPollingNew.push_back(keyCode);
				}
			}
		}

		// OnUpdate
		{
			auto onUpdate = data["OnUpdate"];
			if (onUpdate)
			{
				auto& onUpdateNew = inputMode->m_OnUpdateBindings;
				for (auto binding : onUpdate)
				{
					Input::InputActionTypes bindingType = Utility::StringToInputActionType(binding["BindingType"].as<std::string>());
					Ref<Input::InputActionBinding> newActionBinding = nullptr;
					switch (bindingType)
					{
					case Input::InputActionTypes::KeyboardAction:
					{
						newActionBinding = CreateRef<Input::KeyboardActionBinding>();
						((Input::KeyboardActionBinding*)newActionBinding.get())->SetKeyBinding(binding["KeyBinding"].as<uint32_t>());
						break;
					}
					case Input::InputActionTypes::None:
					default:
					{
						KG_ERROR("Invalid bindingType while deserializing InputMode");
						break;
					}
					}
					Assets::AssetHandle handle = binding["ScriptHandle"].as<uint64_t>();
					if (handle == Assets::EmptyHandle)
					{
						newActionBinding->SetScript(nullptr, Assets::EmptyHandle);
					}
					else
					{
						newActionBinding->SetScript(handle);
					}
					onUpdateNew.push_back(newActionBinding);
				}
			}
		}


		// OnKeyPressed
		{
			auto onKeyPressed = data["CustomCallsOnKeyPressed"];
			if (onKeyPressed)
			{
				auto& onKeyPressedNew = inputMode->m_OnKeyPressedBindings;
				for (auto binding : onKeyPressed)
				{
					Input::InputActionTypes bindingType = Utility::StringToInputActionType(binding["BindingType"].as<std::string>());
					Ref<Input::InputActionBinding> newActionBinding = nullptr;
					switch (bindingType)
					{
					case Input::InputActionTypes::KeyboardAction:
					{
						newActionBinding = CreateRef<Input::KeyboardActionBinding>();
						((Input::KeyboardActionBinding*)newActionBinding.get())->SetKeyBinding(binding["KeyBinding"].as<uint32_t>());
						break;
					}
					case Input::InputActionTypes::None:
					default:
					{
						KG_ERROR("Invalid bindingType while deserializing InputMode");
						break;
					}
					}

					newActionBinding->SetScript(binding["ScriptHandle"].as<uint64_t>());
					onKeyPressedNew.push_back(newActionBinding);
				}
			}
		}

		return true;

	}

	AssetHandle AssetManager::CreateNewInputMode(const std::string& inputModeName)
	{
		// Create Checksum
		const std::string currentCheckSum = Utility::FileSystem::ChecksumFromString(inputModeName);

		if (currentCheckSum.empty())
		{
			KG_ERROR("Failed to generate checksum from file!");
			return {};
		}

		// Compare currentChecksum to registered assets
		for (const auto& [handle, asset] : s_InputModeRegistry)
		{
			if (asset.Data.CheckSum == currentCheckSum)
			{
				KG_INFO("Attempt to instantiate duplicate input asset");
				return handle;
			}
		}

		// Create New Asset/Handle
		AssetHandle newHandle{};
		Assets::Asset newAsset{};
		newAsset.Handle = newHandle;

		// Create File
		CreateInputModeFile(inputModeName, newAsset);
		newAsset.Data.CheckSum = currentCheckSum;

		// Register New Asset and return handle.
		s_InputModeRegistry.insert({ newHandle, newAsset }); // Update Registry Map in-memory
		SerializeInputModeRegistry(); // Update Registry File on Disk

		return newHandle;
	}

	void AssetManager::SaveInputMode(AssetHandle inputModeHandle, Ref<Input::InputMode> inputMode)
	{
		if (!s_InputModeRegistry.contains(inputModeHandle))
		{
			KG_ERROR("Attempt to save inputMode that does not exist in registry");
			return;
		}
		Assets::Asset inputModeAsset = s_InputModeRegistry[inputModeHandle];
		SerializeInputMode(inputMode, (Projects::Project::GetAssetDirectory() / inputModeAsset.Data.IntermediateLocation).string());
	}

	void AssetManager::DeleteInputMode(AssetHandle handle)
	{
		if (!s_InputModeRegistry.contains(handle))
		{
			KG_WARN("Failed to delete input mode in AssetManager");
			return;
		}

		Utility::FileSystem::DeleteSelectedFile(Projects::Project::GetAssetDirectory() /
			s_InputModeRegistry.at(handle).Data.IntermediateLocation);

		s_InputModeRegistry.erase(handle);

		SerializeInputModeRegistry();
	}

	std::filesystem::path AssetManager::GetInputModeLocation(const AssetHandle& handle)
	{
		if (!s_InputModeRegistry.contains(handle))
		{
			KG_ERROR("Attempt to save inputMode that does not exist in registry");
			return "";
		}
		return s_InputModeRegistry[handle].Data.IntermediateLocation;
	}

	Ref<Input::InputMode> AssetManager::GetInputMode(const AssetHandle& handle)
	{
		KG_ASSERT(Projects::Project::GetActive(), "There is no active project when retreiving inputMode!");

		if (s_InputModeRegistry.contains(handle))
		{
			auto asset = s_InputModeRegistry[handle];
			return InstantiateInputMode(asset);
		}

		KG_ERROR("No inputMode is associated with provided handle!");
		return nullptr;
	}
	std::tuple<AssetHandle, Ref<Input::InputMode>> AssetManager::GetInputMode(const std::filesystem::path& filepath)
	{
		KG_ASSERT(Projects::Project::GetActive(), "Attempt to use Project Field without active project!");

		std::filesystem::path inputModePath = filepath;

		if (filepath.is_absolute())
		{
			inputModePath = Utility::FileSystem::GetRelativePath(Projects::Project::GetAssetDirectory(), filepath);
		}

		for (auto& [assetHandle, asset] : s_InputModeRegistry)
		{
			if (asset.Data.IntermediateLocation.compare(inputModePath) == 0)
			{
				return std::make_tuple(assetHandle, InstantiateInputMode(asset));
			}
		}
		// Return empty inputMode if inputMode does not exist
		KG_WARN("No InputMode Associated with provided handle. Returned new empty inputMode");
		AssetHandle newHandle = CreateNewInputMode(filepath.stem().string());
		return std::make_tuple(newHandle, GetInputMode(newHandle));
	}

	Ref<Input::InputMode> AssetManager::InstantiateInputMode(const Assets::Asset& inputModeAsset)
	{
		Ref<Input::InputMode> newInputMode = CreateRef<Input::InputMode>();
		DeserializeInputMode(newInputMode, (Projects::Project::GetAssetDirectory() / inputModeAsset.Data.IntermediateLocation).string());
		return newInputMode;
	}


	void AssetManager::ClearInputModeRegistry()
	{
		s_InputModeRegistry.clear();
	}

	void AssetManager::CreateInputModeFile(const std::string& inputModeName, Assets::Asset& newAsset)
	{
		// Create Temporary InputMode
		Ref<Input::InputMode> temporaryInputMode = CreateRef<Input::InputMode>();

		// Save Binary Intermediate into File
		std::string inputModePath = "Input/" + inputModeName + ".kginput";
		std::filesystem::path intermediateFullPath = Projects::Project::GetAssetDirectory() / inputModePath;
		SerializeInputMode(temporaryInputMode, intermediateFullPath.string());

		// Load data into In-Memory Metadata object
		newAsset.Data.Type = Assets::AssetType::InputMode;
		newAsset.Data.IntermediateLocation = inputModePath;
		Ref<Assets::InputModeMetaData> metadata = CreateRef<Assets::InputModeMetaData>();
		newAsset.Data.SpecificFileData = metadata;
	}
}
