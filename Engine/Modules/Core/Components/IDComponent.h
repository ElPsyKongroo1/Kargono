#pragma once

#include "Kargono/Core/UUID.h"
#include "Modules/ECSInternal/Module/ComponentTag.h"
#include "Modules/Core/Module/CoreModule.h"

namespace Kargono
{
	struct IDComponent
	{
	public:
		//==============================
		// Copy Function(s)
		//==============================
		void CopyTo(IDComponent* dst) 
		{
			// Create the component in place
			std::construct_at<IDComponent>(dst);

			dst->m_ID = m_ID;
		}
	public:
		//==============================
		// Public Fields
		//==============================
		UUID m_ID;
	};

	Register_Module_Type(IDComponent, ECSInternal::ComponentTag)
}