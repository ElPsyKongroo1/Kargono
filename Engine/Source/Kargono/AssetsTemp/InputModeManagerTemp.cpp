#include "kgpch.h"

#include "Kargono/AssetsTemp/AssetService.h"
#include "Kargono/AssetsTemp/InputModeManagerTemp.h"

#include "Kargono/Input/InputMode.h"

namespace Kargono::Assets
{
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
			KG_ERROR("Failed to load .kgui file '{0}'\n     {1}", assetPath, e.what());
			return nullptr;
		}

		KG_INFO("Deserializing input mode");

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
	void InputModeManager::SerializeAssetSpecificMetadata(YAML::Emitter& serializer, Assets::Asset& currentAsset)
	{
	}
	void InputModeManager::DeserializeAssetSpecificMetadata(YAML::Node& metadataNode, Assets::Asset& currentAsset)
	{
		Ref<Assets::InputModeMetaData> inputModeMetaData = CreateRef<Assets::InputModeMetaData>();
		currentAsset.Data.SpecificFileData = inputModeMetaData;
	}
}
