#pragma once

#include "Modules/ECSInternal/Module/ComponentTag.h"
#include "Modules/Core/Module/CoreModule.h"
#include "Kargono/Math/MathAliases.h"

#include <memory>

namespace Kargono
{
	struct Transform
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		Transform() = default;
		Transform(const Math::vec3& translation) : m_Translation(translation) {}

	public:
		//==============================
		// Get as Matrices
		//==============================
		Math::mat4 GetTransform() const;
		Math::mat4 GetTranslation() const;
		Math::mat4 GetRotation() const;

	public:
		//==============================
		// Copy Function(s)
		//==============================
		void CopyTo(Transform* dst)
		{
			// Create the component in place
			std::construct_at<Transform>(dst);

			dst->m_Translation = m_Translation;
			dst->m_Rotation = m_Rotation;
			dst->m_Scale = m_Scale;
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
		Math::vec3 m_Translation{ 0.0f, 0.0f, 0.0f };
		Math::vec3 m_Rotation{ 0.0f, 0.0f, 0.0f };
		Math::vec3 m_Scale{ 1.0f, 1.0f, 1.0f };
	};

	Register_Module_Type(Transform, ECSInternal::ComponentTag)
}