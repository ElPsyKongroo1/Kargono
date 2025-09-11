#pragma once

#include "Modules/Scripting/Module/ScriptingModule.h"
#include "Modules/ECSInternal/Module/ComponentTag.h"

#include "Modules/Assets/Asset.h"
#include "Modules/Scripting/ScriptModuleBinder.h"

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
			// Create the component in place
			std::construct_at<OnUpdateComponent>(dst);

			dst->m_OnUpdateScriptHandle = m_OnUpdateScriptHandle;
			dst->m_OnUpdateScript = m_OnUpdateScript;
		}
	public:
		//==============================
		// Public Fields
		//==============================
		Assets::AssetHandle m_OnUpdateScriptHandle{ Assets::k_EmptyHandle };
		Ref<Scripting::Script> m_OnUpdateScript{ nullptr };
	};

	Register_Module_Type(OnUpdateComponent, ECSInternal::ComponentTag)
}