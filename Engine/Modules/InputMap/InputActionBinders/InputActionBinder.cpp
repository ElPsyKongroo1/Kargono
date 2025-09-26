#include "kgpch.h"

#include "Modules/InputMap/InputActionBinders/InputActionBinder.h"

namespace Kargono::InputMap
{
	void InputActionBinding::SetScript(Assets::AssetHandle handle)
	{
		if (handle == Assets::k_EmptyHandle)
		{
			ClearScript();
			return;
		}

		Ref<Scripting::Script> newScript{ Assets::AssetService::GetScript(handle) };
		KG_ASSERT(newScript);
		m_Script = newScript;
		m_ScriptHandle = handle;
	}
}