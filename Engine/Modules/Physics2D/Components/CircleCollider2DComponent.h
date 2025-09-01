#pragma once

#include "Kargono/Math/MathAliases.h"
#include "Modules/Physics2D/Module/Physics2DModule.h"
#include "Modules/ECSInternal/Module/ComponentTag.h"

namespace Kargono::Physics2D
{
	struct CircleCollider2DComponent
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		CircleCollider2DComponent() = default;
		~CircleCollider2DComponent() = default;

		//==============================
		// Copy Function(s)
		//==============================
		void CopyTo(CircleCollider2DComponent* dst)
		{
			CircleCollider2DComponent* destination = (CircleCollider2DComponent*)dst;
			destination->m_Offset = m_Offset;
			destination->m_Radius = m_Radius;
			destination->m_Density = m_Density;
			destination->m_Friction = m_Friction;
			destination->m_Restitution = m_Restitution;
			destination->m_RestitutionThreshold = m_RestitutionThreshold;
			destination->m_IsSensor = m_IsSensor;
			destination->m_RuntimeFixture = m_RuntimeFixture;
		}
		//==============================
		// Public Fields
		//==============================
		// Dimensions
		Math::vec2 m_Offset{ 0.0f, 0.0f };
		float m_Radius{ 0.5f };
		// Material properties // TODO: Move into physics material
		float m_Density{ 1.0f };
		float m_Friction{ 0.5f };
		float m_Restitution{ 0.0f };
		float m_RestitutionThreshold{ 0.5f };
		bool m_IsSensor{ false };
		// Storage for runtime
		void* m_RuntimeFixture{ nullptr };
	};

	Register_Module_Type(CircleCollider2DComponent, ECSInternal::ComponentTag)
}