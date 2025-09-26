#pragma once

#include "Modules/InputMap/InputActionBinders/InputActionBinder.h"

#include "Kargono/Core/KeyCodes.h"

namespace Kargono::InputMap
{
	class KeyboardActionBinding : public InputActionBinding
	{
	public:
		//=========================
		// Constructors/Destructors
		//=========================
		KeyboardActionBinding();
		virtual ~KeyboardActionBinding() override = default;
	public:
		//=========================
		// Getters/Setters
		//=========================
		KeyCode GetKeyBinding() const { return m_KeyBinding; }
		void SetKeyBinding(KeyCode code) { m_KeyBinding = code; }
	private:
		//=========================
		// Internal Fields
		//=========================
		KeyCode m_KeyBinding{ Key::D0 };
	};
}