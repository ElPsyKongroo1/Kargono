#pragma once

#include "Modules/Scripting/Module/ScriptingModule.h"
#include "Modules/ECSInternal/Module/ComponentTag.h"

#include "Modules/Assets/Asset.h"
#include "Modules/Scripting/ScriptService.h"

namespace Kargono::Scripting
{
	struct OnUpdateComponent
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		OnUpdateComponent() = default;
		~OnUpdateComponent() = default;
	public:
		//==============================
		// Copy Function(s)
		//==============================
		void CopyTo(OnUpdateComponent* dst)
		{
			OnUpdateComponent* destination = (OnUpdateComponent*)dst;
			destination->m_OnUpdateScriptHandle = m_OnUpdateScriptHandle;
			destination->m_OnUpdateScript = m_OnUpdateScript;
		}
	public:
		//==============================
		// Public Fields
		//==============================
		Assets::AssetHandle m_OnUpdateScriptHandle{ Assets::EmptyHandle };
		Ref<Scripting::Script> m_OnUpdateScript{ nullptr };
	};

	Register_Module_Type(OnUpdateComponent, ECSInternal::ComponentTag)
}