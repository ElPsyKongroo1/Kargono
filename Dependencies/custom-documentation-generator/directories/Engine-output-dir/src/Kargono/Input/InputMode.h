#pragma once

#include "Kargono/Core/KeyCodes.h"
#include "Kargono/Core/Base.h"

#include <vector>
#include <unordered_map>
#include <tuple>
#include <functional>

namespace Kargono
{

	class InputMode
	{
	public:

		enum InputActionTypes
		{
			None = 0, KeyboardAction
		};

		class InputActionBinding
		{
		public:
			virtual ~InputActionBinding() = default;
		public:
			virtual void CallFunction() = 0;
		protected:
			virtual void CheckStatus() = 0;
		public:
			InputActionTypes GetActionType() const { return m_BindingType; }
		protected:
			InputActionTypes m_BindingType{ InputActionTypes::None };
		};


		class KeyboardActionBinding : public InputActionBinding
		{
		public:
			KeyboardActionBinding()
				: InputActionBinding()
			{
				m_BindingType = InputActionTypes::KeyboardAction;
			}
			virtual ~KeyboardActionBinding() override = default;
		public:
			virtual void CallFunction() override;
		protected:
			virtual void CheckStatus() override;
		public:
			KeyCode GetKeyBinding() const { return m_KeyBinding; }
			void SetKeyBinding(KeyCode code) { m_KeyBinding = code; }
			std::string& GetFunctionBinding() { return m_FunctionName; }
			void SetFunctionBinding(const std::string& function) { m_FunctionName = function; }
		private:
			KeyCode m_KeyBinding{Key::D0};
			std::string m_FunctionName{"None"};
		};

	public:

		static void AddKeyboardPollingSlot();
		static void UpdateKeyboardPollingSlot(uint16_t originalSlot, uint16_t newSlot);
		static void UpdateKeyboardPollingKey(uint16_t slot, KeyCode newKey);
		static void DeleteKeyboardPollingSlot(uint16_t slot);
		static bool IsKeyboardSlotPressed(uint16_t slot);
		static std::vector<std::tuple<uint16_t, KeyCode>>& GetKeyboardPolling();

		static void AddKeyboardOnUpdateSlot();
		static void DeleteKeyboardCustomCallsOnUpdate(InputMode::InputActionBinding* bindingRef);
		static std::vector<KeyboardActionBinding*>& GetKeyboardCustomCallsOnUpdate();


		static std::vector<std::tuple<std::string, InputMode::KeyboardActionBinding*>>& GetKeyboardClassOnUpdate();


	private:
		// Maps Vector Locations to Engine KeyCodes
		std::unordered_map<uint16_t , KeyCode> m_KeyboardPolling {};

	private:
		// On Event Input -> Action(Function Pointer) Bindings
		std::vector<Ref<InputActionBinding>> m_CustomCallsOnUpdateBindings{};
		std::unordered_map<std::string , std::vector<Ref<InputActionBinding>>> m_ScriptClassOnUpdateBindings{};
		std::unordered_map<std::string, std::vector<Ref<InputActionBinding>>> m_TagOnUpdateBindings{};

	public:
		static InputMode s_InputMode;
	};
}
