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
			// Create the component in place
			std::construct_at<CameraComponent>(dst);

			dst->m_Camera = m_Camera;
		}

	public:
		//==============================
		// Public Fields
		//==============================
		Scenes::SceneCamera m_Camera{};
	};

	Register_Module_Type(CameraComponent, ECSInternal::ComponentTag)
}