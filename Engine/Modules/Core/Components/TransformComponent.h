#pragma once

#include "Modules/ECSInternal/Module/ComponentTag.h"
#include "Modules/Core/Module/CoreModule.h"
#include "Kargono/Math/MathAliases.h"

namespace Kargono
{
	struct TransformComponent
	{
		//==============================
		// Constructors/Destructors
		//==============================
		TransformComponent() = default;
		TransformComponent(const Math::vec3& translation) : m_Translation(translation) {}

		//==============================
		// Get as Matrices
		//==============================
		Math::mat4 GetTransform() const;
		Math::mat4 GetTranslation() const;
		Math::mat4 GetRotation() const;

		//==============================
		// Copy Function(s)
		//==============================
		void CopyTo(TransformComponent* dst)
		{
			// Create the component in place
			std::construct_at<TransformComponent>(dst);

			dst->m_Translation = m_Translation;
			dst->m_Rotation = m_Rotation;
			dst->m_Scale = m_Scale;
		}
		//==============================
		// Public Fields
		//==============================
		Math::vec3 m_Translation{ 0.0f, 0.0f, 0.0f };
		Math::vec3 m_Rotation{ 0.0f, 0.0f, 0.0f };
		Math::vec3 m_Scale{ 1.0f, 1.0f, 1.0f };
	};

	Register_Module_Type(TransformComponent, ECSInternal::ComponentTag)
}