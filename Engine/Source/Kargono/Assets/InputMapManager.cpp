#include "kgpch.h"

#include "Kargono/Assets/AssetService.h"
#include "Kargono/Assets/InputMapManager.h"

#include "Kargono/Input/InputMap.h"

namespace Kargono::Assets
{
	void InputMapManager::CreateAssetFileFromName(std::string_view name, AssetInfo& asset, const std::filesystem::path& assetPath)
	{
		UNREFERENCED_PARAMETER(name);

		// Create Temporary InputMap
		Ref<Input::InputMap> temporaryInputMap = CreateRef<Input::InputMap>();

		// Save Binary into File
		SerializeAsset(temporaryInputMap, assetPath);

		// Load data into In-Memory Metadata object
		Ref<Assets::InputMapMetaData> metadata = CreateRef<Assets::InputMapMetaData>();
		asset.Data.SpecificFileData = metadata;
	}
	void InputMapManager::SerializeAsset(Ref<Input::InputMap> assetReference, const std::filesystem::path& assetPath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map

		{
			// Keyboard Polling
			out << YAML::Key << "KeyboardPolling" << YAML::Value;
			out << YAML::BeginSeq; // Start of KeyboardPolling Seq
			uint32_t iteration{ 0 };
			for (KeyCode keyCode : assetReference->m_KeyboardPolling)
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

			for (Ref<Input::InputActionBinding>& inputBinding : assetReference->m_OnUpdateBindings)
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
					KG_ERROR("Invalid InputMap provided to InputMap serialization");
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

			for (Ref<Input::InputActionBinding>& inputBinding : assetReference->m_OnKeyPressedBindings)
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
					KG_ASSERT("Invalid InputMap provided to InputMap serialization");
					break;
				}
				}

				out << YAML::EndMap; // InputActionBinding End
			}
			out << YAML::EndSeq; // End of OnKeyPressed Seq
		}

		out << YAML::EndMap; // Start of File Map

		std::ofstream fout(assetPath);
		fout << out.c_str();
		KG_INFO("Successfully Serialized InputMap at {}", assetPath.string());
	}
	Ref<Input::InputMap> InputMapManager::DeserializeAsset(Assets::AssetInfo& asset, const std::filesystem::path& assetPath)
	{
		UNREFERENCED_PARAMETER(asset);

		Ref<Input::InputMap> newInputMap = CreateRef<Input::InputMap>();
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

		// Get Keyboard Polling!
		{
			YAML::Node keyboardPolling = data["KeyboardPolling"];
			if (keyboardPolling)
			{
				std::vector<KeyCode>& keyboardPollingNew = newInputMap->m_KeyboardPolling;
				for (const YAML::Node& binding : keyboardPolling)
				{
					uint16_t keyCode = (uint16_t)binding["KeyCode"].as<uint32_t>();
					keyboardPollingNew.emplace_back(keyCode);
				}
			}
		}

		// OnUpdate
		{
			YAML::Node onUpdate = data["OnUpdate"];
			if (onUpdate)
			{
				std::vector<Ref<Input::InputActionBinding>>& onUpdateNew = newInputMap->m_OnUpdateBindings;
				for (const YAML::Node& binding : onUpdate)
				{
					Input::InputActionTypes bindingType = Utility::StringToInputActionType(binding["BindingType"].as<std::string>());
					Ref<Input::InputActionBinding> newActionBinding = nullptr;
					switch (bindingType)
					{
					case Input::InputActionTypes::KeyboardAction:
					{
						newActionBinding = CreateRef<Input::KeyboardActionBinding>();
						((Input::KeyboardActionBinding*)newActionBinding.get())->SetKeyBinding((KeyCode)binding["KeyBinding"].as<uint32_t>());
						break;
					}
					case Input::InputActionTypes::None:
					default:
					{
						KG_ERROR("Invalid bindingType while deserializing InputMap");
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
					onUpdateNew.emplace_back(newActionBinding);
				}
			}
		}


		// OnKeyPressed
		{
			YAML::Node onKeyPressed = data["OnKeyPressed"];
			if (onKeyPressed)
			{
				std::vector<Ref<Input::InputActionBinding>>& onKeyPressedNew = newInputMap->m_OnKeyPressedBindings;
				for (const YAML::Node& binding : onKeyPressed)
				{
					Input::InputActionTypes bindingType = Utility::StringToInputActionType(binding["BindingType"].as<std::string>());
					Ref<Input::InputActionBinding> newActionBinding = nullptr;
					switch (bindingType)
					{
					case Input::InputActionTypes::KeyboardAction:
					{
						newActionBinding = CreateRef<Input::KeyboardActionBinding>();
						((Input::KeyboardActionBinding*)newActionBinding.get())->SetKeyBinding((KeyCode)binding["KeyBinding"].as<uint32_t>());
						break;
					}
					case Input::InputActionTypes::None:
					default:
					{
						KG_ERROR("Invalid bindingType while deserializing InputMap");
						break;
					}
					}

					newActionBinding->SetScript(binding["ScriptHandle"].as<uint64_t>());
					onKeyPressedNew.emplace_back(newActionBinding);
				}
			}
		}

		return newInputMap;
	}
	bool InputMapManager::RemoveScript(Ref<Input::InputMap> inputMapRef, Assets::AssetHandle scriptHandle)
	{
		bool inputModified{ false };
		for (Ref<Input::InputActionBinding> binding : inputMapRef->GetOnUpdateBindings())
		{
			if (binding->GetScriptHandle() == scriptHandle)
			{
				binding->SetScript(nullptr, Assets::EmptyHandle);
				inputModified = true;
			}
		}

		for (Ref<Input::InputActionBinding> binding : inputMapRef->GetOnKeyPressedBindings())
		{
			if (binding->GetScriptHandle() == scriptHandle)
			{
				binding->SetScript(nullptr, Assets::EmptyHandle);
				inputModified = true;
			}
		}
		
		return inputModified;
	}
}
