#pragma once

#include "Modules/ECSInternal/Module/ComponentTag.h"
#include "Modules/Scripting/Module/ScriptingModule.h"
#include "Modules/Assets/TrackedAssetReference.h"

#include "Modules/Assets/AssetsCommon.h"
#include "Modules/Scripting/ScriptModuleBinder.h"

namespace Kargono::Scripting
{
	struct OnCreate
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		OnCreate() = default;
		~OnCreate() = default;
	public:
		//==============================
		// Copy Function(s)
		//==============================
		void CopyTo(OnCreate* dst)
		{
			// Create the component in place
			std::construct_at<OnCreate>(dst);

			dst->m_OnCreateScript = m_OnCreateScript;
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
		Assets::TAssetRef<Scripting::Script> m_OnCreateScript{};
	};

	Register_Module_Type(OnCreate, ECSInternal::ComponentTag)
}