#include "kgpch.h"

#include "Kargono/Assets/AssetService.h"
#include "Kargono/Assets/InputModeManager.h"

#include "Kargono/Input/InputMode.h"

namespace Kargono::Assets
{
	void InputModeManager::CreateAssetFileFromName(const std::string& name, Asset& asset, const std::filesystem::path& assetPath)
	{
		// Create Temporary InputMode
		Ref<Input::InputMode> temporaryInputMode = CreateRef<Input::InputMode>();

		// Save Binary into File
		SerializeAsset(temporaryInputMode, assetPath);

		// Load data into In-Memory Metadata object
		Ref<Assets::InputModeMetaData> metadata = CreateRef<Assets::InputModeMetaData>();
		asset.Data.SpecificFileData = metadata;
	}
	void InputModeManager::SerializeAsset(Ref<Input::InputMode> assetReference, const std::filesystem::path& assetPath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map

		{
			// Keyboard Polling
			out << YAML::Key << "KeyboardPolling" << YAML::Value;
			out << YAML::BeginSeq; // Start of KeyboardPolling Seq
			uint32_t iteration{ 0 };
			for (auto keyCode : assetReference->m_KeyboardPolling)
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

			for (auto& inputBinding : assetReference->m_OnUpdateBindings)
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

			for (auto& inputBinding : assetReference->m_OnKeyPressedBindings)
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

		std::ofstream fout(assetPath);
		fout << out.c_str();
		KG_INFO("Successfully Serialized InputMode at {}", assetPath.string());
	}
	Ref<Input::InputMode> InputModeManager::DeserializeAsset(Assets::Asset& asset, const std::filesystem::path& assetPath)
	{
		Ref<Input::InputMode> newInputMode = CreateRef<Input::InputMode>();
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
			auto keyboardPolling = data["KeyboardPolling"];
			if (keyboardPolling)
			{
				auto& keyboardPollingNew = newInputMode->m_KeyboardPolling;
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
				auto& onUpdateNew = newInputMode->m_OnUpdateBindings;
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
			auto onKeyPressed = data["OnKeyPressed"];
			if (onKeyPressed)
			{
				auto& onKeyPressedNew = newInputMode->m_OnKeyPressedBindings;
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

		return newInputMode;
	}
}
