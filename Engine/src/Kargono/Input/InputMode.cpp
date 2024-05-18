#include "kgpch.h"

#include "Kargono/Input/InputMode.h"
#include "Kargono/Assets/AssetManager.h"

#include "InputPolling.h"
#include "Kargono/Core/EngineCore.h"

namespace Kargono
{
	Ref<InputMode> InputMode::s_InputMode { nullptr };
	Assets::AssetHandle InputMode::s_InputModeHandle {0};

	// Caching for User Interface! TODO: Might want to make this code editor only.
	// This cache only exists so that polling slots can be displayed in order!
	static std::vector<std::tuple<uint16_t, KeyCode>> s_KeyboardPollingCache {};
	static bool s_KeyboardPollingModified = true;

	static std::vector<InputMode::KeyboardActionBinding*> s_CustomCallsKeyboardOnUpdateCache{};
	static bool s_CustomCallsOnUpdateModified = true;

	static std::vector<std::tuple<std::string, InputMode::KeyboardActionBinding*>> s_ClassKeyboardOnUpdateCache{};
	static bool s_ClassOnUpdateModified = true;

	static std::vector<InputMode::KeyboardActionBinding*> s_CustomCallsKeyboardOnKeyPressedCache{};
	static bool s_CustomCallsOnKeyPressedModified = true;

	static std::vector<std::tuple<std::string, InputMode::KeyboardActionBinding*>> s_ClassKeyboardOnKeyPressedCache{};
	static bool s_ClassOnKeyPressedModified = true;

	void InputMode::KeyboardActionBinding::CallFunction()
	{

	}
	void InputMode::KeyboardActionBinding::CheckStatus()
	{
	}
	void InputMode::AddKeyboardPollingSlot()
	{
		s_KeyboardPollingModified = true;
		auto& keyboardPolling = s_InputMode->m_KeyboardPolling;
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
		auto& keyboardPolling = s_InputMode->m_KeyboardPolling;
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
		auto& keyboardPolling = s_InputMode->m_KeyboardPolling;
		keyboardPolling.insert_or_assign(slot, newKey);
	}

	void InputMode::DeleteKeyboardPollingSlot(uint16_t slot)
	{
		s_KeyboardPollingModified = true;
		auto& keyboardPolling = s_InputMode->m_KeyboardPolling;
		if (keyboardPolling.contains(slot))
		{
			keyboardPolling.erase(slot);
		}
	}

	void InputMode::ClearInputEngine()
	{
		s_InputMode = { nullptr };
		s_InputModeHandle = {0};
		s_KeyboardPollingModified = true;
		s_CustomCallsOnUpdateModified = true;
		s_ClassOnUpdateModified = true;
		s_CustomCallsOnKeyPressedModified = true;
		s_ClassOnKeyPressedModified = true;
	}

	void InputMode::LoadInputMode(Ref<InputMode> newInput, Assets::AssetHandle newHandle)
	{
		InputMode::ClearInputEngine();

		s_InputMode = newInput;
		s_InputModeHandle = newHandle;
	}

	void InputMode::LoadInputModeByName(const std::string& inputMode)
	{
		static Ref<InputMode> s_InputRef {nullptr};
		static Assets::AssetHandle s_InputHandle {0};

		auto [handle, inputReference] = Assets::AssetManager::GetInputMode(inputMode);
		s_InputRef = inputReference;
		s_InputHandle = handle;
		if (inputReference)
		{
			EngineCore::GetCurrentApp().SubmitToMainThread([&]()
				{
					LoadInputMode(s_InputRef, s_InputHandle);
				});

		}
	}

	bool InputMode::IsKeyboardSlotPressed(uint16_t slot)
	{
		auto& keyboardPolling = s_InputMode->m_KeyboardPolling;
		if (keyboardPolling.contains(slot))
		{
			return InputPolling::IsKeyPressed(keyboardPolling.at(slot));
		}
		KG_INFO("Atttempt to use key slot that does not exist in current InputMode. The slot is {}", slot);
		return false;
	}

	std::vector<std::tuple<uint16_t, KeyCode>>& InputMode::GetKeyboardPolling()
	{
		if (s_KeyboardPollingModified)
		{
			auto& keyboardPolling = s_InputMode->m_KeyboardPolling;
			std::map<uint16_t, KeyCode> temporaryMap {};
			for (auto& [key, value] : keyboardPolling) { temporaryMap.insert({ key, value }); }
			s_KeyboardPollingCache.clear();
			for (auto& [key, value] : temporaryMap){ s_KeyboardPollingCache.push_back(std::make_tuple(key, value)); }
			s_KeyboardPollingModified = false;
		}
		return s_KeyboardPollingCache;
	}

	void InputMode::AddKeyboardCustomCallsOnUpdateSlot()
	{
		s_CustomCallsOnUpdateModified = true;
		s_InputMode->m_CustomCallsOnUpdateBindings.push_back(CreateRef<KeyboardActionBinding>());
	}

	void InputMode::AddKeyboardCustomCallsOnKeyPressedSlot()
	{
		s_CustomCallsOnKeyPressedModified = true;
		s_InputMode->m_CustomCallsOnKeyPressedBindings.push_back(CreateRef<KeyboardActionBinding>());
	}

	void InputMode::UpdateKeyboardClassOnUpdateName(InputMode::InputActionBinding* bindingRef, const std::string& newClassName)
	{
		s_ClassOnUpdateModified = true;
		auto& classOnUpdate = s_InputMode->m_ScriptClassOnUpdateBindings;


		uint32_t iterator{ 0 };
		uint32_t refSlot{ 0 };
		std::string className {};
		bool refFound = false;
		Ref<InputActionBinding> binding { nullptr };
		for (auto& [name, list] : classOnUpdate)
		{
			for (auto& reference : list)
			{
				if (reference.get() == bindingRef)
				{
					refFound = true;
					refSlot = iterator;
					className = name;
					binding = reference;
				}
			}
		}

		if (!refFound)
		{
			KG_ERROR("Iterator not found inside UpdateKeyboardClassOnUpdateName");
			return;
		}

		if (!classOnUpdate.contains(newClassName))
		{
			classOnUpdate.insert({ newClassName, {} });
		}

		classOnUpdate.at(newClassName).push_back(binding);

		auto& bindingVector = classOnUpdate.at(className);

		bindingVector.erase(bindingVector.begin() + refSlot);
		if (bindingVector.empty()) { classOnUpdate.erase(className); }
	}

	void InputMode::UpdateKeyboardClassOnKeyPressedName(InputMode::InputActionBinding* bindingRef, const std::string& newClassName)
	{
		s_ClassOnKeyPressedModified = true;
		auto& classOnKeyPressed = s_InputMode->m_ScriptClassOnKeyPressedBindings;


		uint32_t iterator{ 0 };
		uint32_t refSlot{ 0 };
		std::string className {};
		bool refFound = false;
		Ref<InputActionBinding> binding { nullptr };
		for (auto& [name, list] : classOnKeyPressed)
		{
			for (auto& reference : list)
			{
				if (reference.get() == bindingRef)
				{
					refFound = true;
					refSlot = iterator;
					className = name;
					binding = reference;
				}
			}
		}

		if (!refFound)
		{
			KG_ERROR("Iterator not found inside UpdateKeyboardClassOnKeyPressedName");
			return;
		}

		if (!classOnKeyPressed.contains(newClassName))
		{
			classOnKeyPressed.insert({ newClassName, {} });
		}

		classOnKeyPressed.at(newClassName).push_back(binding);

		auto& bindingVector = classOnKeyPressed.at(className);

		bindingVector.erase(bindingVector.begin() + refSlot);
		if (bindingVector.empty()) { classOnKeyPressed.erase(className); }
	}

	void InputMode::AddKeyboardScriptClassOnUpdateSlot()
	{
		s_ClassOnUpdateModified = true;
		auto& scriptMap = s_InputMode->m_ScriptClassOnUpdateBindings;
		if (!scriptMap.contains("None"))
		{
			scriptMap.insert({"None", {}});
			scriptMap.at("None").push_back(CreateRef<KeyboardActionBinding>());
			return;
		}

		scriptMap.at("None").push_back(CreateRef<KeyboardActionBinding>());
	}

	void InputMode::AddKeyboardScriptClassOnKeyPressedSlot()
	{
		s_ClassOnKeyPressedModified = true;
		auto& scriptMap = s_InputMode->m_ScriptClassOnKeyPressedBindings;
		if (!scriptMap.contains("None"))
		{
			scriptMap.insert({ "None", {} });
			scriptMap.at("None").push_back(CreateRef<KeyboardActionBinding>());
			return;
		}

		scriptMap.at("None").push_back(CreateRef<KeyboardActionBinding>());
	}


	std::vector<InputMode::KeyboardActionBinding*>& InputMode::GetKeyboardCustomCallsOnUpdate()
	{
		if (s_CustomCallsOnUpdateModified)
		{
			s_CustomCallsKeyboardOnUpdateCache.clear();
			for (auto& binding : s_InputMode->m_CustomCallsOnUpdateBindings)
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

	std::vector<InputMode::KeyboardActionBinding*>& InputMode::GetKeyboardCustomCallsOnKeyPressed()
	{
		if (s_CustomCallsOnKeyPressedModified)
		{
			s_CustomCallsKeyboardOnKeyPressedCache.clear();
			for (auto& binding : s_InputMode->m_CustomCallsOnKeyPressedBindings)
			{
				if (binding->GetActionType() == InputMode::KeyboardAction)
				{
					s_CustomCallsKeyboardOnKeyPressedCache.push_back((InputMode::KeyboardActionBinding*)binding.get());
				}
			}

			// TODO: Ensure you update all other caches as well!!!!!
			s_CustomCallsOnKeyPressedModified = false;
		}
		return s_CustomCallsKeyboardOnKeyPressedCache;
	}

	

	std::vector<std::tuple<std::string, InputMode::KeyboardActionBinding*>>& InputMode::GetKeyboardClassOnUpdate()
	{
		if (s_ClassOnUpdateModified)
		{
			s_ClassKeyboardOnUpdateCache.clear();
			for (auto& [className, classBindings] : s_InputMode->m_ScriptClassOnUpdateBindings)
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

	std::vector<std::tuple<std::string, InputMode::KeyboardActionBinding*>>& InputMode::GetKeyboardClassOnKeyPressed()
	{
		if (s_ClassOnKeyPressedModified)
		{
			s_ClassKeyboardOnKeyPressedCache.clear();
			for (auto& [className, classBindings] : s_InputMode->m_ScriptClassOnKeyPressedBindings)
			{
				for (auto& binding : classBindings)
				{
					if (binding->GetActionType() == InputMode::KeyboardAction)
					{
						s_ClassKeyboardOnKeyPressedCache.push_back(std::make_tuple(className, (InputMode::KeyboardActionBinding*)binding.get()));
					}
				}
			}
			// TODO: Ensure you update all other caches as well!!!!!
			s_ClassOnKeyPressedModified = false;
		}
		return s_ClassKeyboardOnKeyPressedCache;
	}
	

	std::vector<Ref<InputMode::InputActionBinding>>& InputMode::GetCustomCallsOnUpdate()
	{
		return s_InputMode->m_CustomCallsOnUpdateBindings;
	}

	std::unordered_map<std::string, std::vector<Ref<InputMode::InputActionBinding>>>& InputMode::GetScriptClassOnUpdate()
	{
		return s_InputMode->m_ScriptClassOnUpdateBindings;
	}

	std::vector<Ref<InputMode::InputActionBinding>>& InputMode::GetCustomCallsOnKeyPressed()
	{
		return s_InputMode->m_CustomCallsOnKeyPressedBindings;
	}

	std::unordered_map<std::string, std::vector<Ref<InputMode::InputActionBinding>>>& InputMode::GetScriptClassOnKeyPressed()
	{
		return s_InputMode->m_ScriptClassOnKeyPressedBindings;
	}

	void InputMode::DeleteKeyboardCustomCallsOnUpdate(InputMode::InputActionBinding* bindingRef)
	{
		s_CustomCallsOnUpdateModified = true;
		auto& customCallsOnUpdate = s_InputMode->m_CustomCallsOnUpdateBindings;
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
			KG_ERROR("Iterator not found inside DeleteKeyboardCustomCallsOnUpdate");
			return;
		}
		
		customCallsOnUpdate.erase(customCallsOnUpdate.begin() + refSlot);
	}

	void InputMode::DeleteKeyboardCustomCallsOnKeyPressed(InputMode::InputActionBinding* bindingRef)
	{
		s_CustomCallsOnKeyPressedModified = true;
		auto& customCallsOnKeyPressed = s_InputMode->m_CustomCallsOnKeyPressedBindings;
		uint32_t iterator{ 0 };
		uint32_t refSlot{ 0 };
		bool refFound = false;
		for (auto& reference : customCallsOnKeyPressed)
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
			KG_ERROR("Iterator not found inside DeleteKeyboardCustomCallsOnKeyPressed");
			return;
		}

		customCallsOnKeyPressed.erase(customCallsOnKeyPressed.begin() + refSlot);
	}
	


	void InputMode::DeleteKeyboardScriptClassOnUpdate(InputMode::InputActionBinding* bindingRef)
	{
		s_ClassOnUpdateModified = true;
		auto& classOnUpdate = s_InputMode->m_ScriptClassOnUpdateBindings;


		uint32_t iterator{ 0 };
		uint32_t refSlot{ 0 };
		std::string className {};
		bool refFound = false;
		for (auto& [name, list] : classOnUpdate)
		{
			for (auto& reference :list)
			{
				if (reference.get() == bindingRef)
				{
					refFound = true;
					refSlot = iterator;
					className = name;
				}
			}
		}

		if (!refFound)
		{
			KG_ERROR("Iterator not found inside DeleteKeyboardScriptClassOnUpdate");
			return;
		}

		auto& bindingVector = classOnUpdate.at(className);

		bindingVector.erase(bindingVector.begin() + refSlot);
		if (bindingVector.empty()){ classOnUpdate.erase(className); }
	}

	void InputMode::DeleteKeyboardScriptClassOnKeyPressed(InputMode::InputActionBinding* bindingRef)
	{
		s_ClassOnKeyPressedModified = true;
		auto& classOnKeyPressed = s_InputMode->m_ScriptClassOnKeyPressedBindings;


		uint32_t iterator{ 0 };
		uint32_t refSlot{ 0 };
		std::string className {};
		bool refFound = false;
		for (auto& [name, list] : classOnKeyPressed)
		{
			for (auto& reference : list)
			{
				if (reference.get() == bindingRef)
				{
					refFound = true;
					refSlot = iterator;
					className = name;
				}
			}
		}

		if (!refFound)
		{
			KG_ERROR("Iterator not found inside DeleteKeyboardScriptClassOnKeyPressed");
			return;
		}

		auto& bindingVector = classOnKeyPressed.at(className);

		bindingVector.erase(bindingVector.begin() + refSlot);
		if (bindingVector.empty()) { classOnKeyPressed.erase(className); }
	}
}
