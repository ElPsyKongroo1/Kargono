#pragma once

#include "Modules/ECSInternal/Module/ComponentTag.h"
#include "Modules/Cameras/Module/CamerasModule.h"

#include "Modules/Cameras/StaticCamera.h"

namespace Kargono::Cameras
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
		// Serialization
		//==============================
		void Serialize(void* context);
		void Deserialize(void* context);
	public:
		//==============================
		// Public Fields
		//==============================
		StaticCamera m_Camera{};
	};

	Register_Module_Type(CameraComponent, ECSInternal::ComponentTag)
}