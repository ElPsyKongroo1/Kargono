#include "kgpch.h"

#include "Modules/InputMap/InputActionBinders/KeyboardActionBinder.h"

namespace Kargono::InputMap
{
	KeyboardActionBinding::KeyboardActionBinding()
		: InputActionBinding()
	{
		m_BindingType = InputActionTypes::KeyboardAction;
	}
}