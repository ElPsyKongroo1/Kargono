#pragma once
#include <glm/glm.hpp>

namespace Kargono
{
	class Camera
	{
	public:
		Camera(const glm::mat4& projection)
			:m_Projection(projection) {}

		const glm::mat4& GetProjection() const { return m_Projection; }

		// TODO: Something
	private:
		glm::mat4 m_Projection;
	};
}