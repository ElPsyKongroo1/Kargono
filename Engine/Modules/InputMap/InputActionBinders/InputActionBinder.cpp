#include "kgpch.h"

#include "Modules/InputMap/InputActionBinders/InputActionBinder.h"
#include "Modules/Assets/Managers/ScriptManager.h"

namespace Kargono::InputMap
{
	void InputActionBinding::SetScript(Assets::AssetHandle handle)
	{
		if (handle == Assets::k_EmptyHandle)
		{
			ClearScript();
			return;
		}

		Assets::AssetRef<Scripting::Script> newScript{ Assets::s_ScriptManager.GetAssetByHandle(handle) };
		KG_ASSERT(newScript);
		m_Script = newScript;
		m_ScriptHandle = handle;
	}
}