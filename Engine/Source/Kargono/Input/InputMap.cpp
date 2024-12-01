#include "kgpch.h"

#include "Kargono/Input/InputMap.h"
#include "Kargono/Scenes/Scene.h"

#include "Kargono/Input/InputService.h"
#include "Kargono/Core/Engine.h"
#include "Kargono/Assets/AssetService.h"

namespace Kargono::Input
{
	Ref<InputMap> InputMapService::s_ActiveInputMap { nullptr };
	Assets::AssetHandle InputMapService::s_ActiveInputMapHandle {0};

	void InputMapService::ClearActiveInputMap()
	{
		s_ActiveInputMap = { nullptr };
		s_ActiveInputMapHandle = {0};
	}

	void InputMapService::SetActiveInputMap(Ref<InputMap> newInput, Assets::AssetHandle newHandle)
	{
		s_ActiveInputMap = newInput;
		s_ActiveInputMapHandle = newHandle;
	}

	void InputMapService::SetActiveInputMapByName(const std::string& inputMap)
	{
		static Ref<InputMap> s_InputRef {nullptr};
		static Assets::AssetHandle s_InputHandle {0};

		auto [handle, inputReference] = Assets::AssetService::GetInputMap(inputMap);
		s_InputRef = inputReference;
		s_InputHandle = handle;

		if (!inputReference)
		{
			KG_WARN("Input map is invalid. Failed to set active input map by name!");
			return;
		}

		EngineService::SubmitToMainThread([&]()
		{
			SetActiveInputMap(s_InputRef, s_InputHandle);
		});
		
	}

	void InputMapService::OnUpdate(Timestep ts)
	{
		if (Input::InputMapService::GetActiveInputMap())
		{
			for (Ref<InputActionBinding> inputBinding : Input::InputMapService::GetActiveOnUpdate())
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
		}
	}
	bool InputMapService::OnKeyPressed(Events::KeyPressedEvent event)
	{
		if (event.IsRepeat()) { return false; }
		if (Input::InputMapService::GetActiveInputMap())
		{
			for (Ref<InputActionBinding> inputBinding : Input::InputMapService::GetActiveOnKeyPressed())
			{
				
				Input::KeyboardActionBinding* keyboardBinding = (Input::KeyboardActionBinding*)inputBinding.get();
				if (!Input::InputService::IsKeyPressed(keyboardBinding->GetKeyBinding()) || 
					keyboardBinding->GetScriptHandle() == Assets::EmptyHandle) 
				{ 
					continue; 
				}
				Utility::CallWrappedVoidNone(keyboardBinding->GetScript()->m_Function);
			}
		}

		return false;
	}

	bool InputMapService::IsPollingSlotPressed(uint16_t slot)
	{
		if (!s_ActiveInputMap)
		{
			KG_WARN("Attempt to query keyboard slot from active input map, however, no active input map exists");
			return false;
		}
		std::vector<KeyCode>& keyboardPolling = s_ActiveInputMap->GetKeyboardPolling();
		if (slot >= (uint16_t)keyboardPolling.size())
		{
			return false;
		}

		return InputService::IsKeyPressed(keyboardPolling.at(slot));
	}
	std::vector<Ref<InputActionBinding>>& InputMapService::GetActiveOnUpdate()
	{
		KG_ASSERT(s_ActiveInputMap);
		return s_ActiveInputMap->GetOnUpdateBindings();
	}
	std::vector<Ref<InputActionBinding>>& InputMapService::GetActiveOnKeyPressed()
	{
		KG_ASSERT(s_ActiveInputMap);
		return s_ActiveInputMap->GetOnKeyPressedBindings();
	}
	void InputActionBinding::SetScript(Assets::AssetHandle handle)
	{
		if (handle == Assets::EmptyHandle)
		{
			ClearScript();
			return;
		}

		Ref<Scripting::Script> newScript = Assets::AssetService::GetScript(handle);
		KG_ASSERT(newScript);
		m_Script = newScript;
		m_ScriptHandle = handle;
	}
}
