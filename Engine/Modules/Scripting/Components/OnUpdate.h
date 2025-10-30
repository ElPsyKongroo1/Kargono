#pragma once

#include "Modules/Scripting/Module/ScriptingModule.h"
#include "Modules/ECSInternal/Module/ComponentTag.h"

#include "Modules/Assets/AssetsCommon.h"
#include "Modules/Scripting/ScriptModuleBinder.h"

namespace Kargono::Scripting
{
	struct OnUpdate
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		OnUpdate() = default;
		~OnUpdate() = default;
	public:
		//==============================
		// Copy Function(s)
		//==============================
		void CopyTo(OnUpdate* dst)
		{
			// Create the component in place
			std::construct_at<OnUpdate>(dst);

			dst->m_OnUpdateScriptHandle = m_OnUpdateScriptHandle;
			dst->m_OnUpdateScript = m_OnUpdateScript;
		}
	public:
		//==============================
		// Serialization
		//==============================
		void Serialize(void* context);
		void Deserialize(void* context);
	public:
		//==============================
		// Public Fields
		//==============================
		Assets::AssetHandle m_OnUpdateScriptHandle{ Assets::k_EmptyHandle };
		Ref<Scripting::Script> m_OnUpdateScript{ nullptr };
	};

	Register_Module_Type(OnUpdate, ECSInternal::ComponentTag)
}