#include "kgpch.h"

#include "Kargono/Input/InputMode.h"

#include "InputPolling.h"

namespace Kargono
{
	InputMode InputMode::s_InputMode {};

	// Caching for User Interface! TODO: Might want to make this code editor only.
	static std::vector<std::tuple<uint16_t, KeyCode>> s_KeyboardPollingCache {};
	static bool s_KeyboardPollingModified = true;

	static std::vector<InputMode::KeyboardActionBinding*> s_CustomCallsKeyboardOnUpdateCache{};
	static bool s_CustomCallsOnUpdateModified = true;

	static std::vector<std::tuple<std::string, InputMode::KeyboardActionBinding*>> s_ClassKeyboardOnUpdateCache{};
	static bool s_ClassOnUpdateModified = true;

	void InputMode::KeyboardActionBinding::CallFunction()
	{

	}
	void InputMode::KeyboardActionBinding::CheckStatus()
	{
	}
	void InputMode::AddKeyboardPollingSlot()
	{
		s_KeyboardPollingModified = true;
		auto& keyboardPolling = s_InputMode.m_KeyboardPolling;
		for (uint16_t iterator{0}; iterator < 120; iterator++)
		{
			if (!keyboardPolling.contains(iterator))
			{
				keyboardPolling.insert({ iterator, Key::D0 });
				return;
			}
		}
	}
	void InputMode::UpdateKeyboardPollingSlot(uint16_t originalSlot ,uint16_t newSlot)
	{
		s_KeyboardPollingModified = true;
		auto& keyboardPolling = s_InputMode.m_KeyboardPolling;
		KeyCode keyCode{ Key::D0 };
		if (keyboardPolling.contains(originalSlot))
		{
			keyCode = keyboardPolling.at(originalSlot);
			keyboardPolling.erase(originalSlot);
		}

		keyboardPolling.insert({ newSlot, keyCode });
	}
	void InputMode::UpdateKeyboardPollingKey(uint16_t slot, KeyCode newKey)
	{
		s_KeyboardPollingModified = true;
		auto& keyboardPolling = s_InputMode.m_KeyboardPolling;
		keyboardPolling.insert_or_assign(slot, newKey);
	}

	void InputMode::DeleteKeyboardPollingSlot(uint16_t slot)
	{
		s_KeyboardPollingModified = true;
		auto& keyboardPolling = s_InputMode.m_KeyboardPolling;
		if (keyboardPolling.contains(slot))
		{
			keyboardPolling.erase(slot);
		}
	}

	bool InputMode::IsKeyboardSlotPressed(uint16_t slot)
	{
		auto& keyboardPolling = s_InputMode.m_KeyboardPolling;
		if (keyboardPolling.contains(slot))
		{
			return InputPolling::IsKeyPressed(keyboardPolling.at(slot));
		}
		KG_CORE_TRACE("Atttempt to use key slot that does not exist in current InputMode. The slot is {}", slot);
		return false;
	}

	std::vector<std::tuple<uint16_t, KeyCode>>& InputMode::GetKeyboardPolling()
	{
		if (s_KeyboardPollingModified)
		{
			auto& keyboardPolling = s_InputMode.m_KeyboardPolling;
			std::map<uint16_t, KeyCode> temporaryMap {};
			for (auto& [key, value] : keyboardPolling) { temporaryMap.insert({ key, value }); }
			s_KeyboardPollingCache.clear();
			for (auto& [key, value] : temporaryMap){ s_KeyboardPollingCache.push_back(std::make_tuple(key, value)); }
			s_KeyboardPollingModified = false;
		}
		return s_KeyboardPollingCache;
	}

	void InputMode::AddKeyboardOnUpdateSlot()
	{
		s_CustomCallsOnUpdateModified = true;
		s_InputMode.m_CustomCallsOnUpdateBindings.push_back(CreateRef<KeyboardActionBinding>());
	}


	std::vector<InputMode::KeyboardActionBinding*>& InputMode::GetKeyboardCustomCallsOnUpdate()
	{
		if (s_CustomCallsOnUpdateModified)
		{
			s_CustomCallsKeyboardOnUpdateCache.clear();
			for (auto& binding : s_InputMode.m_CustomCallsOnUpdateBindings)
			{
				if (binding->GetActionType() == InputMode::KeyboardAction)
				{
					s_CustomCallsKeyboardOnUpdateCache.push_back((InputMode::KeyboardActionBinding*)binding.get());
				}
			}

			// TODO: Ensure you update all other caches as well!!!!!
			s_CustomCallsOnUpdateModified = false;
		}
		return s_CustomCallsKeyboardOnUpdateCache;
	}

	std::vector<std::tuple<std::string, InputMode::KeyboardActionBinding*>>& InputMode::GetKeyboardClassOnUpdate()
	{
		if (s_ClassOnUpdateModified)
		{
			s_ClassKeyboardOnUpdateCache.clear();
			for (auto& [className, classBindings] : s_InputMode.m_ScriptClassOnUpdateBindings)
			{
				for (auto& binding: classBindings)
				{
					if (binding->GetActionType() == InputMode::KeyboardAction)
					{
						s_ClassKeyboardOnUpdateCache.push_back(std::make_tuple(className, (InputMode::KeyboardActionBinding*)binding.get()));
					}
				}
			}
			// TODO: Ensure you update all other caches as well!!!!!
			s_ClassOnUpdateModified = false;
		}
		return s_ClassKeyboardOnUpdateCache;
	}

	void InputMode::DeleteKeyboardCustomCallsOnUpdate(InputMode::InputActionBinding* bindingRef)
	{
		s_CustomCallsOnUpdateModified = true;
		auto& customCallsOnUpdate = s_InputMode.m_CustomCallsOnUpdateBindings;
		uint32_t iterator{ 0 };
		uint32_t refSlot{ 0 };
		bool refFound = false;
		for (auto& reference  : customCallsOnUpdate)
		{
			if (reference.get() == bindingRef)
			{
				refFound = true;
				refSlot = iterator;
			}
			iterator++;
		}

		if (!refFound)
		{
			KG_CORE_ERROR("Iterator not found inside DeleteKeyboardCustomCallsOnUpdate");
			return;
		}
		
		customCallsOnUpdate.erase(customCallsOnUpdate.begin() + refSlot);
	}
}
