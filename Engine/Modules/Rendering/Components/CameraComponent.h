#pragma once

#include "Modules/ECSInternal/Module/ComponentTag.h"
#include "Modules/Rendering/Module/RenderingModule.h"

#include "Kargono/Scenes/SceneCamera.h"

namespace Kargono::Rendering
{
	struct CameraComponent
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		CameraComponent() = default;
		~CameraComponent() = default;

	public:
		//==============================
		// Copy Function(s)
		//==============================
		void CopyTo(CameraComponent* dst)
		{
			CameraComponent* destination = (CameraComponent*)dst;
			destination->m_Camera = m_Camera;
			destination->m_Primary = m_Primary;
		}

	public:
		//==============================
		// Public Fields
		//==============================
		Scenes::SceneCamera m_Camera{};
		bool m_Primary{ true };
	};

	Register_Module_Type(CameraComponent, ECSInternal::ComponentTag)
}