#include "kgpch.h"

#include "Kargono/Input/InputMode.h"

#include "Kargono/Input/InputPolling.h"
#include "Kargono/Core/EngineCore.h"
#include "Kargono/Assets/AssetManager.h"

namespace Kargono::Input
{
	Ref<InputMode> InputModeEngine::s_ActiveInputMode { nullptr };
	Assets::AssetHandle InputModeEngine::s_ActiveInputModeHandle {0};

	void InputModeEngine::ClearActiveInputMode()
	{
		s_ActiveInputMode = { nullptr };
		s_ActiveInputModeHandle = {0};
	}

	void InputModeEngine::SetActiveInputMode(Ref<InputMode> newInput, Assets::AssetHandle newHandle)
	{
		s_ActiveInputMode = newInput;
		s_ActiveInputModeHandle = newHandle;
	}

	void InputModeEngine::SetActiveInputModeByName(const std::string& inputMode)
	{
		static Ref<InputMode> s_InputRef {nullptr};
		static Assets::AssetHandle s_InputHandle {0};

		auto [handle, inputReference] = Assets::AssetManager::GetInputMode(inputMode);
		s_InputRef = inputReference;
		s_InputHandle = handle;
		if (inputReference)
		{
			EngineCore::GetCurrentEngineCore().SubmitToMainThread([&]()
			{
				SetActiveInputMode(s_InputRef, s_InputHandle);
			});
		}
	}

	bool InputModeEngine::IsActiveKeyboardSlotPressed(uint16_t slot)
	{
		KG_ASSERT(s_ActiveInputMode);
		auto& keyboardPolling = s_ActiveInputMode->GetKeyboardPolling();
		if (keyboardPolling.contains(slot))
		{
			return InputPolling::IsKeyPressed(keyboardPolling.at(slot));
		}
		KG_WARN("Atttempt to use key slot that does not exist in current InputMode. The slot is {}", slot);
		return false;
	}
	std::vector<Ref<InputActionBinding>>& InputModeEngine::GetActiveOnUpdate()
	{
		KG_ASSERT(s_ActiveInputMode);
		return s_ActiveInputMode->GetOnUpdateBindings();
	}
	std::vector<Ref<InputActionBinding>>& InputModeEngine::GetActiveOnKeyPressed()
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
