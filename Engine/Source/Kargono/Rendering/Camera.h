#pragma once

#include "Kargono/Math/Math.h"

namespace Kargono::Rendering
{
	class Camera
	{
	public:
		Camera() = default;
		Camera(const Math::mat4& projection)
			:m_Projection(projection) {}

		virtual ~Camera() = default;

		const Math::mat4& GetProjection() const { return m_Projection; }

		// TODO: Something
	protected:
		Math::mat4 m_Projection = Math::mat4(1.0f);
	};
}
