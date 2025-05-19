#include "kgpch.h"

#include "Modules/InputMap/InputMap.h"
#include "Kargono/Scenes/Scene.h"

#include "Modules/Input/InputService.h"
#include "Modules/Core/Engine.h"
#include "Modules/Assets/AssetService.h"

namespace Kargono::Input
{
	void InputMapContext::ClearActiveInputMap()
	{
		m_ActiveInputMap = { nullptr };
		m_ActiveInputMapHandle = {0};
	}

	void InputMapContext::SetActiveInputMap(Ref<InputMap> newInput, Assets::AssetHandle newHandle)
	{
		m_ActiveInputMap = newInput;
		m_ActiveInputMapHandle = newHandle;
	}

	void InputMapContext::SetActiveInputMapFromHandle(Assets::AssetHandle inputMapHandle)
	{
		static Ref<InputMap> s_InputRef {nullptr};
		static Assets::AssetHandle s_InputHandle {0};

		Ref<Input::InputMap> inputReference = Assets::AssetService::GetInputMap(inputMapHandle);
		s_InputRef = inputReference;
		s_InputHandle = inputMapHandle;

		if (!inputReference)
		{
			KG_WARN("Input map is invalid. Failed to set active input map by name!");
			return;
		}

		EngineService::GetActiveEngine().GetThread().SubmitFunction([&]()
		{
			SetActiveInputMap(s_InputRef, s_InputHandle);
		});
		
	}

	bool InputMapContext::Init()
	{
		return true;
	}

	bool InputMapContext::Terminate()
	{
		return true;
	}

	void InputMapContext::OnUpdate(Timestep ts)
	{
		if (Input::InputMapContext::GetActiveInputMap())
		{
			for (Ref<InputActionBinding> inputBinding : Input::InputMapContext::GetActiveOnUpdate())
			{
				
				Input::KeyboardActionBinding* keyboardBinding = (Input::KeyboardActionBinding*)inputBinding.get();
				KG_ASSERT(keyboardBinding->GetScript());
				if (!Input::InputService::IsKeyPressed(keyboardBinding->GetKeyBinding()) ||
					keyboardBinding->GetScriptHandle() == Assets::EmptyHandle)
				{ 
					continue; 
				}
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
	bool InputMapContext::OnKeyPressed(Events::KeyPressedEvent event)
	{
		if (event.IsRepeat()) { return false; }
		if (Input::InputMapContext::GetActiveInputMap())
		{
			for (Ref<InputActionBinding> inputBinding : Input::InputMapContext::GetActiveOnKeyPressed())
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

	bool InputMapContext::IsPollingSlotPressed(uint16_t slot)
	{
		if (!m_ActiveInputMap)
		{
			KG_WARN("Attempt to query keyboard slot from active input map, however, no active input map exists");
			return false;
		}
		std::vector<KeyCode>& keyboardPolling = m_ActiveInputMap->GetKeyboardPolling();
		if (slot >= (uint16_t)keyboardPolling.size())
		{
			return false;
		}

		return InputService::IsKeyPressed(keyboardPolling.at(slot));
	}
	std::vector<Ref<InputActionBinding>>& InputMapContext::GetActiveOnUpdate()
	{
		KG_ASSERT(m_ActiveInputMap);
		return m_ActiveInputMap->GetOnUpdateBindings();
	}
	std::vector<Ref<InputActionBinding>>& InputMapContext::GetActiveOnKeyPressed()
	{
		KG_ASSERT(m_ActiveInputMap);
		return m_ActiveInputMap->GetOnKeyPressedBindings();
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
