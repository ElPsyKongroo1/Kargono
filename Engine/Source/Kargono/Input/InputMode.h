#pragma once

#include "Kargono/Core/KeyCodes.h"
#include "Kargono/Core/Base.h"
#include "Kargono/Assets/Asset.h"

#include <vector>
#include <unordered_map>
#include <tuple>
#include <functional>
#include <string>

namespace Kargono::Assets
{
	class AssetManager;
}

namespace Kargono::Input
{

	enum InputActionTypes
	{
		None = 0, KeyboardAction = 1
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
		std::string& GetFunctionBinding() { return m_FunctionName; }
		void SetFunctionBinding(const std::string& function) { m_FunctionName = function; }
	protected:
		InputActionTypes m_BindingType{ InputActionTypes::None };
		std::string m_FunctionName{"None"};
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

	private:
		KeyCode m_KeyBinding{ Key::D0 };
	};

	class InputMode
	{
	public:
		static void ClearInputEngine();

		static void LoadInputMode(Ref<InputMode> newInput, Assets::AssetHandle newHandle);
		static void LoadInputModeByName(const std::string& inputMode);
	public:
		//=========================
		// Editor Management Functions
		//=========================
		static void AddKeyboardPollingSlot();
		static void UpdateKeyboardPollingSlot(uint16_t originalSlot, uint16_t newSlot);
		static void UpdateKeyboardPollingKey(uint16_t slot, KeyCode newKey);
		static void DeleteKeyboardPollingSlot(uint16_t slot);
		static std::vector<std::tuple<uint16_t, KeyCode>>& GetKeyboardPolling();

		static void AddKeyboardCustomCallsOnUpdateSlot();
		static void DeleteKeyboardCustomCallsOnUpdate(InputActionBinding* bindingRef);
		static std::vector<KeyboardActionBinding*>& GetKeyboardCustomCallsOnUpdate();

		static void AddKeyboardScriptClassOnUpdateSlot();
		static void DeleteKeyboardScriptClassOnUpdate(InputActionBinding* bindingRef);
		static void UpdateKeyboardClassOnUpdateName(InputActionBinding* bindingRef, const std::string& newClassName);
		static std::vector<std::tuple<std::string, KeyboardActionBinding*>>& GetKeyboardClassOnUpdate();

		static void AddKeyboardCustomCallsOnKeyPressedSlot();
		static void DeleteKeyboardCustomCallsOnKeyPressed(InputActionBinding* bindingRef);
		static std::vector<KeyboardActionBinding*>& GetKeyboardCustomCallsOnKeyPressed();

		static void AddKeyboardScriptClassOnKeyPressedSlot();
		static void DeleteKeyboardScriptClassOnKeyPressed(InputActionBinding* bindingRef);
		static void UpdateKeyboardClassOnKeyPressedName(InputActionBinding* bindingRef, const std::string& newClassName);
		static std::vector<std::tuple<std::string, KeyboardActionBinding*>>& GetKeyboardClassOnKeyPressed();


		//=========================
		// Engine Getter Functions
		//=========================
		static bool IsKeyboardSlotPressed(uint16_t slot);
		static std::vector<Ref<InputActionBinding>>& GetCustomCallsOnUpdate();
		static std::unordered_map<std::string, std::vector<Ref<InputActionBinding>>>& GetScriptClassOnUpdate();

		static std::vector<Ref<InputActionBinding>>& GetCustomCallsOnKeyPressed();
		static std::unordered_map<std::string, std::vector<Ref<InputActionBinding>>>& GetScriptClassOnKeyPressed();


	private:
		// Maps Vector Locations to Engine KeyCodes
		std::unordered_map<uint16_t, KeyCode> m_KeyboardPolling {};

	private:
		// On Event Input -> Action(Function Pointer) Bindings
		std::vector<Ref<InputActionBinding>> m_CustomCallsOnUpdateBindings{};
		std::unordered_map<std::string , std::vector<Ref<InputActionBinding>>> m_ScriptClassOnUpdateBindings{};

		std::vector<Ref<InputActionBinding>> m_CustomCallsOnKeyPressedBindings{};
		std::unordered_map<std::string, std::vector<Ref<InputActionBinding>>> m_ScriptClassOnKeyPressedBindings{};

	public:
		static Ref<InputMode> s_InputMode;
		static Assets::AssetHandle s_InputModeHandle;
		friend class Assets::AssetManager;
	};
}


namespace Kargono::Utility
{
	//==============================
	// InputMode::InputActionTypes <-> String Conversions
	//==============================
	inline std::string InputActionTypeToString(Input::InputActionTypes type)
	{
		switch (type)
		{
		case Input::InputActionTypes::KeyboardAction: return "KeyboardAction";
		case Input::InputActionTypes::None: return "None";
		}
		KG_ERROR("Unknown Type of InputMode::InputActionTypes.");
		return "";
	}

	inline Input::InputActionTypes StringToInputActionType(std::string type)
	{
		if (type == "KeyboardAction") { return Input::InputActionTypes::KeyboardAction; }
		if (type == "None") { return Input::InputActionTypes::None; }

		KG_ERROR("Unknown Type of InputMode::InputActionTypes String.");
		return Input::InputActionTypes::None;
	}
}
