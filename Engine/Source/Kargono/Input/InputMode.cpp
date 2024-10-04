#include "kgpch.h"

#include "Kargono/Input/InputMode.h"
#include "Kargono/Scenes/Scene.h"

#include "Kargono/Input/InputService.h"
#include "Kargono/Core/Engine.h"
#include "Kargono/Assets/AssetService.h"

namespace Kargono::Input
{
	Ref<InputMode> InputModeService::s_ActiveInputMode { nullptr };
	Assets::AssetHandle InputModeService::s_ActiveInputModeHandle {0};

	void InputModeService::ClearActiveInputMode()
	{
		s_ActiveInputMode = { nullptr };
		s_ActiveInputModeHandle = {0};
	}

	void InputModeService::SetActiveInputMode(Ref<InputMode> newInput, Assets::AssetHandle newHandle)
	{
		s_ActiveInputMode = newInput;
		s_ActiveInputModeHandle = newHandle;
	}

	void InputModeService::SetActiveInputModeByName(const std::string& inputMode)
	{
		static Ref<InputMode> s_InputRef {nullptr};
		static Assets::AssetHandle s_InputHandle {0};

		auto [handle, inputReference] = Assets::AssetService::GetInputMode(inputMode);
		s_InputRef = inputReference;
		s_InputHandle = handle;

		if (!inputReference)
		{
			KG_WARN("Input mode is invalid. Failed to set active input mode by name!");
			return;
		}

		EngineService::SubmitToMainThread([&]()
		{
			SetActiveInputMode(s_InputRef, s_InputHandle);
		});
		
	}

	void InputModeService::OnUpdate(Timestep ts)
	{
		if (Input::InputModeService::GetActiveInputMode())
		{
			for (auto& inputBinding : Input::InputModeService::GetActiveOnUpdate())
			{
				if (inputBinding->GetScript()->m_ScriptType != Scripting::ScriptType::Class)
				{
					Input::KeyboardActionBinding* keyboardBinding = (Input::KeyboardActionBinding*)inputBinding.get();
					KG_ASSERT(keyboardBinding->GetScript());
					if (!Input::InputService::IsKeyPressed(keyboardBinding->GetKeyBinding())) { continue; }
					if (keyboardBinding->GetScript()->m_FuncType == WrappedFuncType::Void_None)
					{
						Utility::CallWrappedVoidNone(keyboardBinding->GetScript()->m_Function);
					}
					else
					{
						Utility::CallWrappedVoidFloat(keyboardBinding->GetScript()->m_Function, ts);
					}
				}
				else
				{
					Ref<Scenes::Scene> activeScene = Scenes::SceneService::GetActiveScene();
					KG_ASSERT(activeScene, "No active scene provided while processing input mode on update function");
					Input::KeyboardActionBinding* keyboardBinding = (Input::KeyboardActionBinding*)inputBinding.get();
					if (!Input::InputService::IsKeyPressed(keyboardBinding->GetKeyBinding())) { continue; }
					Ref<Scripting::Script> script = keyboardBinding->GetScript();
					KG_ASSERT(script);
					KG_ASSERT(activeScene->m_ScriptClassToEntityList.contains(keyboardBinding->GetScript()->m_SectionLabel));
					for (auto entity : activeScene->m_ScriptClassToEntityList.at(keyboardBinding->GetScript()->m_SectionLabel))
					{
						if (keyboardBinding->GetScript()->m_FuncType == WrappedFuncType::Void_UInt64)
						{
							Utility::CallWrappedVoidUInt64(keyboardBinding->GetScript()->m_Function, entity);
						}
						else
						{
							Utility::CallWrappedVoidUInt64Float(keyboardBinding->GetScript()->m_Function, entity, ts);
						}
					}
				}
			}
		}
	}
	bool InputModeService::OnKeyPressed(Events::KeyPressedEvent event)
	{
		if (event.IsRepeat()) { return false; }
		if (Input::InputModeService::GetActiveInputMode())
		{
			for (auto& inputBinding : Input::InputModeService::GetActiveOnKeyPressed())
			{
				if (inputBinding->GetScript()->m_ScriptType != Scripting::ScriptType::Class)
				{
					Input::KeyboardActionBinding* keyboardBinding = (Input::KeyboardActionBinding*)inputBinding.get();
					if (!Input::InputService::IsKeyPressed(keyboardBinding->GetKeyBinding())) { continue; }
					Utility::CallWrappedVoidNone(keyboardBinding->GetScript()->m_Function);
				}
			}
		}

		return false;
	}

	bool InputModeService::IsActiveKeyboardSlotPressed(uint16_t slot)
	{
		KG_ASSERT(s_ActiveInputMode);
		auto& keyboardPolling = s_ActiveInputMode->GetKeyboardPolling();
		KG_ASSERT(slot < (uint16_t)keyboardPolling.size(), "Invalid range provided to function");
		return InputService::IsKeyPressed(keyboardPolling.at(slot));
	}
	std::vector<Ref<InputActionBinding>>& InputModeService::GetActiveOnUpdate()
	{
		KG_ASSERT(s_ActiveInputMode);
		return s_ActiveInputMode->GetOnUpdateBindings();
	}
	std::vector<Ref<InputActionBinding>>& InputModeService::GetActiveOnKeyPressed()
	{
		KG_ASSERT(s_ActiveInputMode);
		return s_ActiveInputMode->GetOnKeyPressedBindings();
	}
	void InputActionBinding::SetScript(Assets::AssetHandle handle)
	{
		Ref<Scripting::Script> newScript = Assets::AssetService::GetScript(handle);
		KG_ASSERT(newScript);
		m_Script = newScript;
		m_ScriptHandle = handle;
	}
}
