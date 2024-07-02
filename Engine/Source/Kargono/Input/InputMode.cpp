#include "kgpch.h"

#include "Kargono/Input/InputMode.h"

#include "Kargono/Input/InputPolling.h"
#include "Kargono/Core/Engine.h"
#include "Kargono/Assets/AssetManager.h"

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

		auto [handle, inputReference] = Assets::AssetManager::GetInputMode(inputMode);
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

	bool InputModeService::IsActiveKeyboardSlotPressed(uint16_t slot)
	{
		KG_ASSERT(s_ActiveInputMode);
		auto& keyboardPolling = s_ActiveInputMode->GetKeyboardPolling();
		KG_ASSERT(slot < (uint16_t)keyboardPolling.size(), "Invalid range provided to function");
		return InputPolling::IsKeyPressed(keyboardPolling.at(slot));
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
		Ref<Scripting::Script> newScript = Assets::AssetManager::GetScript(handle);
		KG_ASSERT(newScript);
		m_Script = newScript;
		m_ScriptHandle = handle;
	}
}
