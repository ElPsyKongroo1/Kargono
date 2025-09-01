#pragma once

#include "Modules/ECSInternal/Module/ComponentTag.h"
#include "Modules/Scripting/Module/ScriptingModule.h"

#include "Modules/Assets/Asset.h"
#include "Modules/Scripting/ScriptService.h"

namespace Kargono::Scripting
{
	struct OnCreateComponent
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		OnCreateComponent() = default;
		~OnCreateComponent() = default;
	public:
		//==============================
		// Copy Function(s)
		//==============================
		void CopyTo(OnCreateComponent* dst)
		{
			OnCreateComponent* destination = (OnCreateComponent*)dst;
			destination->m_OnCreateScriptHandle = m_OnCreateScriptHandle;
			destination->m_OnCreateScript = m_OnCreateScript;
		}
	public:
		//==============================
		// Public Fields
		//==============================
		Assets::AssetHandle m_OnCreateScriptHandle{ Assets::EmptyHandle };
		Ref<Scripting::Script> m_OnCreateScript{ nullptr };
	};

	Register_Module_Type(OnCreateComponent, ECSInternal::ComponentTag)
}