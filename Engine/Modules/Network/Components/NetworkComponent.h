#pragma once

#include "Modules/Network/Module/NetworkModule.h"
#include "Modules/ECSInternal/Module/ComponentTag.h"

namespace Kargono::Network
{
	enum class NetworkType
	{
		None = 0,
		ClientAuthoritative,
		ServerTime,
		TrustLastCollision
	};

	struct NetworkComponent
	{
	public:
		//==============================
		// Copy Function(s)
		//==============================
		void CopyTo(NetworkComponent* dst)
		{
			// Create the component in place
			std::construct_at<NetworkComponent>(dst);

			dst->m_NetworkPhysics = m_NetworkPhysics;
			dst->m_PhysicsType = m_PhysicsType;
		}
	public:
		//==============================
		// Public Fields
		//==============================
		bool m_NetworkPhysics{ false };
		NetworkType m_PhysicsType{ NetworkType::None };
	};

	Register_Module_Type(NetworkComponent, ECSInternal::ComponentTag)
}