#include "kgpch.h"

#include "Modules/InputMap/Assets/InputMap.h"
#include "Kargono/Scenes/Scene.h"

#include "Modules/Input/InputService.h"
#include "Modules/Core/Engine.h"
#include "Modules/Assets/AssetService.h"

namespace Kargono::InputMap
{
	void InputMap::Serialize(void* context)
	{
		// Get asset context
		Assets::SerializeAssetContext* assetContext = (Assets::SerializeAssetContext*)context;
		KG_ASSERT(assetContext, "Context cannot be null");
		Assets::Metadata* metadata{ assetContext->m_AssetMetadata };
		KG_ASSERT(metadata, "Metadata cannot be null");

		// Get context fields
		const std::filesystem::path& assetPath{ metadata->GetAssetFullFilePath<InputMap>() };

		// Serialize
		YAML::Emitter out;
		out << YAML::BeginMap; // Start of File Map

		{
			// Keyboard Polling
			out << YAML::Key << "KeyboardPolling" << YAML::Value;
			out << YAML::BeginSeq; // Start of KeyboardPolling Seq
			uint32_t iteration{ 0 };
			for (KeyCode keyCode : m_KeyboardPolling)
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

			for (Ref<InputActionBinding>& inputBinding : m_OnUpdateBindings)
			{
				out << YAML::BeginMap; // InputActionBinding Start

				out << YAML::Key << "BindingType" << YAML::Value << Utility::InputActionTypeToString(inputBinding->GetActionType());
				out << YAML::Key << "ScriptHandle" << YAML::Value << static_cast<uint64_t>(inputBinding->GetScriptHandle());

				switch (inputBinding->GetActionType())
				{
				case InputActionTypes::KeyboardAction:
				{
					KeyboardActionBinding* keyboardBinding = (KeyboardActionBinding*)inputBinding.get();
					out << YAML::Key << "KeyBinding" << YAML::Value << keyboardBinding->GetKeyBinding();
					break;
				}
				case InputActionTypes::None:
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

			for (Ref<InputActionBinding>& inputBinding : m_OnKeyPressedBindings)
			{
				out << YAML::BeginMap; // InputActionBinding Start

				out << YAML::Key << "BindingType" << YAML::Value << Utility::InputActionTypeToString(inputBinding->GetActionType());
				out << YAML::Key << "ScriptHandle" << YAML::Value << static_cast<uint64_t>(inputBinding->GetScriptHandle());

				switch (inputBinding->GetActionType())
				{
				case InputActionTypes::KeyboardAction:
				{
					KeyboardActionBinding* keyboardBinding = (KeyboardActionBinding*)inputBinding.get();
					out << YAML::Key << "KeyBinding" << YAML::Value << keyboardBinding->GetKeyBinding();
					break;
				}
				case InputActionTypes::None:
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

	void InputMap::Deserialize(void* context)
	{
		KG_ASSERT(context, "Context cannot be null");

		// Get asset context
		Assets::DeserializeAssetContext* assetContext = (Assets::DeserializeAssetContext*)context;
		KG_ASSERT(assetContext, "Context cannot be null");
		Assets::Metadata* metadata{ assetContext->m_AssetMetadata };
		KG_ASSERT(metadata, "Metadata cannot be null");

		// Get context fields
		const std::filesystem::path& assetPath{ metadata->GetAssetFullFilePath<InputMap>() };

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

		// Get Keyboard Polling!
		{
			YAML::Node keyboardPolling = data["KeyboardPolling"];
			if (keyboardPolling)
			{
				std::vector<KeyCode>& keyboardPollingNew = m_KeyboardPolling;
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
				std::vector<Ref<InputActionBinding>>& onUpdateNew = m_OnUpdateBindings;
				for (const YAML::Node& binding : onUpdate)
				{
					InputActionTypes bindingType = Utility::StringToInputActionType(binding["BindingType"].as<std::string>());
					Ref<InputActionBinding> newActionBinding = nullptr;
					switch (bindingType)
					{
					case InputActionTypes::KeyboardAction:
					{
						newActionBinding = CreateRef<KeyboardActionBinding>();
						((KeyboardActionBinding*)newActionBinding.get())->SetKeyBinding((KeyCode)binding["KeyBinding"].as<uint32_t>());
						break;
					}
					case InputActionTypes::None:
					default:
					{
						KG_ERROR("Invalid bindingType while deserializing InputMap");
						break;
					}
					}
					Assets::AssetHandle handle = binding["ScriptHandle"].as<uint64_t>();
					if (handle == Assets::k_EmptyHandle)
					{
						newActionBinding->SetScript(nullptr, Assets::k_EmptyHandle);
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
				std::vector<Ref<InputActionBinding>>& onKeyPressedNew = m_OnKeyPressedBindings;
				for (const YAML::Node& binding : onKeyPressed)
				{
					InputActionTypes bindingType = Utility::StringToInputActionType(binding["BindingType"].as<std::string>());
					Ref<InputActionBinding> newActionBinding = nullptr;
					switch (bindingType)
					{
					case InputActionTypes::KeyboardAction:
					{
						newActionBinding = CreateRef<KeyboardActionBinding>();
						((KeyboardActionBinding*)newActionBinding.get())->SetKeyBinding((KeyCode)binding["KeyBinding"].as<uint32_t>());
						break;
					}
					case InputActionTypes::None:
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
	}

	void InputMap::CreateAssetFromName(Assets::Metadata& metadata)
	{
		// Create default input map
		InputMap defaultInputMap{};

		// Save binary into file
		Assets::SerializeAssetContext context{ &metadata };
		defaultInputMap.Serialize((void*)&context);
	}

	bool InputMap::RemoveScript(Assets::AssetHandle scriptHandle)
	{
		bool inputModified{ false };
		for (Ref<InputActionBinding> binding : GetOnUpdateBindings())
		{
			if (binding->GetScriptHandle() == scriptHandle)
			{
				binding->SetScript(nullptr, Assets::k_EmptyHandle);
				inputModified = true;
			}
		}

		for (Ref<InputActionBinding> binding : GetOnKeyPressedBindings())
		{
			if (binding->GetScriptHandle() == scriptHandle)
			{
				binding->SetScript(nullptr, Assets::k_EmptyHandle);
				inputModified = true;
			}
		}

		return inputModified;
	}
}
