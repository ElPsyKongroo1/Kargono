#pragma once
#include "Kargono/Core/UUID.h"
#include "Kargono/Math/MathAliases.h"

namespace Kargono::Physics
{
	struct RaycastResult
	{
		bool m_Success;
		UUID m_Entity;
		Math::vec2 m_Normal;
		Math::vec2 m_Location;
	};
}
