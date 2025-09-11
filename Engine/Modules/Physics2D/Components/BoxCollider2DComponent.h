#pragma once

#include "Kargono/Math/MathAliases.h"
#include "Modules/Physics2D/Module/Physics2DModule.h"
#include "Modules/ECSInternal/Module/ComponentTag.h"


namespace Kargono::Physics2D
{
	struct BoxCollider2DComponent
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		BoxCollider2DComponent() = default;
		~BoxCollider2DComponent() = default;

		//==============================
		// Copy Function(s)
		//==============================
		void CopyTo(BoxCollider2DComponent* dst)
		{
			// Create the component in place
			std::construct_at<BoxCollider2DComponent>(dst);

			// Copy data
			dst->m_Offset = m_Offset;
			dst->m_Size = m_Size;
			dst->m_Density = m_Density;
			dst->m_Friction = m_Friction;
			dst->m_Restitution = m_Restitution;
			dst->m_RestitutionThreshold = m_RestitutionThreshold;
			dst->m_IsSensor = m_IsSensor;
			dst->m_RuntimeFixture = m_RuntimeFixture;
		}
	public:
		//==============================
		// Public Fields
		// ==============================
		// Dimensions
		Math::vec2 m_Offset = { 0.0f, 0.0f };
		Math::vec2 m_Size = { 0.5f, 0.5f };
		// Material properties // TODO: Move into physics material
		float m_Density{ 1.0f };
		float m_Friction{ 0.5f };
		float m_Restitution{ 0.0f };
		float m_RestitutionThreshold{ 0.5f };
		bool m_IsSensor{ false };
		// Storage for runtime
		void* m_RuntimeFixture{ nullptr };
	};

	Register_Module_Type(BoxCollider2DComponent, ECSInternal::ComponentTag)
}