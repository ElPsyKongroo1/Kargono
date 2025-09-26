#pragma once

#include "Kargono/Core/Base.h"
#include "Modules/Scripting/ScriptModuleBinder.h"
#include "Modules/Assets/AssetsCommon.h"

namespace Kargono::InputMap
{
	enum InputActionTypes
	{
		None = 0, KeyboardAction = 1
	};

	class InputActionBinding
	{
	public:
		//=========================
		// Constructors/Destructors
		//=========================
		InputActionBinding() = default;
		virtual ~InputActionBinding() = default;
	public:
		//=========================
		// Getters/Setters
		//=========================
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

		void ClearScript()
		{
			m_ScriptHandle = Assets::k_EmptyHandle;
			m_Script = nullptr;
		}

		void SetScript(Ref<Scripting::Script> script, Assets::AssetHandle handle)
		{
			if (handle == Assets::k_EmptyHandle)
			{
				ClearScript();
				return;
			}

			m_ScriptHandle = handle;
			m_Script = script;
		}
		void SetScript(Assets::AssetHandle handle);
	protected:
		//=========================
		// Protected Fields
		//=========================
		InputActionTypes m_BindingType{ InputActionTypes::None };
		Assets::AssetHandle m_ScriptHandle{ Assets::k_EmptyHandle };
		Ref<Scripting::Script> m_Script{ nullptr };
	};
}