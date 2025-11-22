#pragma once
#include "kgpch.h"

#include "Modules/Assets/Managers/InputMapManager.h"

#include "Modules/InputMap/InputMapContext.h"
#include "Modules/Input/InputService.h"
#include "Modules/Core/Engine.h"
#include "Modules/Scripting/Assets/Script.h"

namespace Kargono::InputMap
{
	void InputMapContext::ClearActiveInputMap()
	{
		m_ActiveInputMap.Reset();
	}

	void InputMapContext::SetActiveInputMap(Assets::AssetRef<InputMap> newInput)
	{
		m_ActiveInputMap = newInput;
	}

	void InputMapContext::SetActiveInputMapFromHandle(Assets::AssetHandle inputMapHandle)
	{
		static Assets::AssetRef<InputMap> s_InputRef{};

	    Assets::AssetRef<InputMap> inputReference = Assets::s_InputMapManager.GetAssetByHandle(inputMapHandle);
		s_InputRef = inputReference;

		if (!inputReference)
		{
			KG_WARN("Input map is invalid. Failed to set active input map by name!");
			return;
		}

		EngineService::GetActiveEngine().GetThread().SubmitFunction([&]()
		{
			SetActiveInputMap(s_InputRef);
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
		if (InputMapContext::GetActiveInputMap())
		{
			for (Ref<InputActionBinding> inputBinding : InputMapContext::GetActiveOnUpdate())
			{

				KeyboardActionBinding* keyboardBinding = (KeyboardActionBinding*)inputBinding.get();
				KG_ASSERT(keyboardBinding->GetScript());
				if (!Input::InputService::IsKeyPressed(keyboardBinding->GetKeyBinding()) ||
					keyboardBinding->GetScriptHandle() == Assets::k_EmptyHandle)
				{
					continue;
				}
				if (keyboardBinding->GetScript()->m_FuncType == WrappedFuncType::Void_None)
				{
					Utility::CallWrapped<WrappedVoidNone>(keyboardBinding->GetScript()->m_Function);
				}
				else
				{
					Utility::CallWrapped<WrappedVoidFloat>(keyboardBinding->GetScript()->m_Function, ts);
				}

			}
		}
	}
	bool InputMapContext::OnKeyPressed(Events::KeyPressedEvent event)
	{
		if (event.IsRepeat()) { return false; }
		if (InputMapContext::GetActiveInputMap())
		{
			for (Ref<InputActionBinding> inputBinding : InputMapContext::GetActiveOnKeyPressed())
			{

				KeyboardActionBinding* keyboardBinding = (KeyboardActionBinding*)inputBinding.get();
				if (!Input::InputService::IsKeyPressed(keyboardBinding->GetKeyBinding()) ||
					keyboardBinding->GetScriptHandle() == Assets::k_EmptyHandle)
				{
					continue;
				}
				Utility::CallWrapped<WrappedVoidNone>(keyboardBinding->GetScript()->m_Function);
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

		return Input::InputService::IsKeyPressed(keyboardPolling.at(slot));
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
}