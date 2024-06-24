#pragma once

#include "Kargono/Core/KeyCodes.h"
#include "Kargono/Core/Base.h"
#include "Kargono/Assets/Asset.h"
#include "Kargono/Scripting/Scripting.h"

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
	//=========================
	// Input Type Defines
	//=========================
	enum InputActionTypes
	{
		None = 0, KeyboardAction = 1
	};

	//=========================
	// Input Action Bindings (i.e. Bind Input -> Function Call)
	//=========================
	class InputActionBinding
	{
	public:
		virtual ~InputActionBinding() = default;
	public:
		InputActionTypes GetActionType() const
		{
			return m_BindingType;
		}
		Ref<Scripting::Script> GetScript()
		{
			return m_Script;
		}
		Assets::AssetHandle GetScriptHandle() const
		{
			return m_ScriptHandle;
		}
		void SetScript(Ref<Scripting::Script> script, Assets::AssetHandle handle)
		{
			m_ScriptHandle = handle;
			m_Script = script;
		}
		void SetScript(Assets::AssetHandle handle);
	protected:
		InputActionTypes m_BindingType{ InputActionTypes::None };
		Assets::AssetHandle m_ScriptHandle { Assets::EmptyHandle };
		Ref<Scripting::Script> m_Script { nullptr };
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
		KeyCode GetKeyBinding() const { return m_KeyBinding; }
		void SetKeyBinding(KeyCode code) { m_KeyBinding = code; }

	private:
		KeyCode m_KeyBinding{ Key::D0 };
	};
	//=========================
	// Input Mode Class
	//=========================
	class InputMode
	{
	public:
		//=========================
		// Getters/Setters
		//=========================
		std::vector<Ref<InputActionBinding>>& GetOnUpdateBindings()
		{
			return m_OnUpdateBindings;
		}
		std::vector<Ref<InputActionBinding>>& GetOnKeyPressedBindings()
		{
			return m_OnKeyPressedBindings;
		}
		std::vector<KeyCode>& GetKeyboardPolling()
		{
			return m_KeyboardPolling;
		}
	private:
		//=========================
		// Input Polling Bindings
		//=========================
		// Maps Vector Locations to Engine KeyCodes
		std::vector<KeyCode> m_KeyboardPolling {};

		//=========================
		// OnEvent -> Action Bindings
		//=========================
		std::vector<Ref<InputActionBinding>> m_OnUpdateBindings{};
		std::vector<Ref<InputActionBinding>> m_OnKeyPressedBindings{};
	private:
		friend class Assets::AssetManager;
	};

	class InputModeService
	{
	public:
		
		//=========================
		// Active Input Mode API
		//=========================
		static bool IsActiveKeyboardSlotPressed(uint16_t slot);
		static std::vector<Ref<InputActionBinding>>& GetActiveOnUpdate();
		static std::vector<Ref<InputActionBinding>>& GetActiveOnKeyPressed();

		//=========================
		// Getter/Setter
		//=========================
		static void ClearActiveInputMode();
		static void SetActiveInputMode(Ref<InputMode> newInput, Assets::AssetHandle newHandle);
		static void SetActiveInputModeByName(const std::string& inputMode);
		static Ref<InputMode> GetActiveInputMode()
		{
			return s_ActiveInputMode;
		}
		static Assets::AssetHandle GetActiveInputModeHandle()
		{
			return s_ActiveInputModeHandle;
		}
	private:
		//=========================
		// Current Input Mode
		//=========================
		static Ref<InputMode> s_ActiveInputMode;
		static Assets::AssetHandle s_ActiveInputModeHandle;
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
