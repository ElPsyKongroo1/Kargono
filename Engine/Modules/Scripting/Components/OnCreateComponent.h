#pragma once

#include "Modules/ECSInternal/Module/ComponentTag.h"
#include "Modules/Scripting/Module/ScriptingModule.h"

#include "Modules/Assets/Asset.h"
#include "Modules/Scripting/ScriptModuleBinder.h"

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
			// Create the component in place
			std::construct_at<OnCreateComponent>(dst);

			dst->m_OnCreateScriptHandle = m_OnCreateScriptHandle;
			dst->m_OnCreateScript = m_OnCreateScript;
		}
	public:
		//==============================
		// Public Fields
		//==============================
		Assets::AssetHandle m_OnCreateScriptHandle{ Assets::k_EmptyHandle };
		Ref<Scripting::Script> m_OnCreateScript{ nullptr };
	};

	Register_Module_Type(OnCreateComponent, ECSInternal::ComponentTag)
}