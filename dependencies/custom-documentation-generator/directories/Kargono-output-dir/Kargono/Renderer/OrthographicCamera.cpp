#include "Kargono/kgpch.h"
#include "OrthographicCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Kargono
{
/// @brief Constructor for the Orthographic Camera class
	OrthographicCamera::OrthographicCamera(float left, float right, float bottom, float top)
		: m_ProjectionMatrix(glm::ortho(left,right, bottom, top, -1.0f, 1.0f)), m_ViewMatrix(1.0f)
	{
/// @details Updates the view-projection matrix by multiplying the projection matrix with the view matrix
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}
/// @brief Recalculates the view matrix based on the camera's position and rotation
	void OrthographicCamera::RecalculateViewMatrix()
	{
/// @details Calculates the transformation matrix using translation and rotation
		glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position) * 
			glm::rotate(glm::mat4(1.0f), m_Rotation, glm::vec3(0, 0, 1));
/// @details Calculates the inverse of the transformation matrix as the view matrix
		m_ViewMatrix = glm::inverse(transform);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}
}