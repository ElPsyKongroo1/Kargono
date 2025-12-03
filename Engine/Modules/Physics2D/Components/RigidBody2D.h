#pragma once

#include "Modules/Assets/AssetsCommon.h"
#include "Modules/Scripting/ScriptModuleBinder.h"
#include "Modules/Assets/TrackedAssetReference.h"
#include "Modules/Scripting/Assets/Script.h"

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

	struct RigidBody2D
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		RigidBody2D() = default;
		~RigidBody2D() = default;
	public:
		//==============================
		// Copy Function(s)
		//==============================
		void CopyTo(RigidBody2D* dst)
		{
			// Create the component in place
			std::construct_at<RigidBody2D>(dst);

			// Copy fields
			dst->m_Type = m_Type;
			dst->m_FixedRotation = m_FixedRotation;
			dst->m_OnCollisionStartScript = m_OnCollisionStartScript;
			dst->m_OnCollisionEndScript = m_OnCollisionEndScript;
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
		// Physics material properties
		BodyType m_Type{ BodyType::Static };
		bool m_FixedRotation{ false };
		// Storage for runtime
		void* m_RuntimeBody{ nullptr };
		// Collision callback function pointers
		Assets::TAssetRef<Scripting::Script> m_OnCollisionStartScript{};
		Assets::TAssetRef<Scripting::Script> m_OnCollisionEndScript{};
	};

	Register_Module_Type(RigidBody2D, ECSInternal::ComponentTag)
}

namespace Kargono::Utility
{
	// These are simply here to help with serialization for the rigid body components in an entity
	inline const char* RigidBody2DBodyTypeToString(Physics2D::BodyType bodyType)
	{
		switch (bodyType)
		{
		case Physics2D::BodyType::Static:	return "Static";
		case Physics2D::BodyType::Dynamic:	return "Dynamic";
		case Physics2D::BodyType::Kinematic:	return "Kinematic";
		}

		KG_ERROR("Unknown body type")
			return {};
	}

	inline Physics2D::BodyType StringToRigidBody2DBodyType(std::string_view bodyTypeString)
	{
		if (bodyTypeString == "Static") return Physics2D::BodyType::Static;
		if (bodyTypeString == "Dynamic") return Physics2D::BodyType::Dynamic;
		if (bodyTypeString == "Kinematic") return Physics2D::BodyType::Kinematic;

		KG_ERROR("Unknown body type")
			return Physics2D::BodyType::Static;
	}
}