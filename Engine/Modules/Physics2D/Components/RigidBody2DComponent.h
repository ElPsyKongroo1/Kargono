#pragma once

#include "Modules/Assets/Asset.h"
#include "Modules/Scripting/ScriptModuleBinder.h"

#include "Modules/Physics2D/Module/Physics2DModule.h"
#include "Modules/ECSInternal/Module/ComponentTag.h"

namespace Kargono::Physics2D
{
	enum class BodyType
	{
		Static = 0,
		Dynamic,
		Kinematic
	};

	struct Rigidbody2DComponent
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		Rigidbody2DComponent() = default;
		~Rigidbody2DComponent() = default;

	public:
		//==============================
		// Copy Function(s)
		//==============================
		void CopyTo(Rigidbody2DComponent* dst)
		{
			// Create the component in place
			std::construct_at<Rigidbody2DComponent>(dst);

			// Copy fields
			dst->m_Type = m_Type;
			dst->m_FixedRotation = m_FixedRotation;
			dst->m_OnCollisionStartScriptHandle = m_OnCollisionStartScriptHandle;
			dst->m_OnCollisionStartScript = m_OnCollisionStartScript;
			dst->m_OnCollisionEndScriptHandle = m_OnCollisionEndScriptHandle;
			dst->m_OnCollisionEndScript = m_OnCollisionEndScript;
		}

	public:
		//==============================
		// Public Fields
		//==============================
		// Physics material properties
		BodyType m_Type{ BodyType::Static };
		bool m_FixedRotation{ false };
		// Storage for runtime
		void* m_RuntimeBody{ nullptr };
		// Collision callback function pointers
		Assets::AssetHandle m_OnCollisionStartScriptHandle{ Assets::k_EmptyHandle };
		Ref<Scripting::Script> m_OnCollisionStartScript{ nullptr };
		Assets::AssetHandle m_OnCollisionEndScriptHandle{ Assets::k_EmptyHandle };
		Ref<Scripting::Script> m_OnCollisionEndScript{ nullptr };
	};

	Register_Module_Type(Rigidbody2DComponent, ECSInternal::ComponentTag)
}