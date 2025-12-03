#pragma once

#include "Kargono/Core/Base.h"
#include "Modules/Scripting/ScriptModuleBinder.h"
#include "Modules/Assets/AssetsCommon.h"
#include "Modules/Assets/TrackedAssetReference.h"

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
		Assets::AssetRef<Scripting::Script> GetScript()
		{
			return m_Script.GetAssetRef();
		}
		Assets::AssetHandle GetScriptHandle() const
		{
			return m_Script.GetAssetRef().GetHandle();
		}

		void ClearScript()
		{
			m_Script.Reset();
		}

		void SetScript(Assets::AssetRef<Scripting::Script> script)
		{
			if (script.IsEmpty())
			{
				ClearScript();
				return;
			}

			m_Script = script;
		}
		void SetScript(Assets::AssetHandle handle);
	protected:
		//=========================
		// Protected Fields
		//=========================
		InputActionTypes m_BindingType{ InputActionTypes::None };
		Assets::TAssetRef<Scripting::Script> m_Script{};
	};
}