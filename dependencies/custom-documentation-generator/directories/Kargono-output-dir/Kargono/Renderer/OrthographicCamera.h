#pragma once
#include <glm/glm.hpp>
/// @namespace Kargono
namespace Kargono
{
/// @class OrthographicCamera
	class OrthographicCamera
	{
	public:
/// @brief Constructor for creating an OrthographicCamera object
/// @param left The left edge of the camera frustum
/// @param right The right edge of the camera frustum
/// @param bottom The bottom edge of the camera frustum
/// @param top The top edge of the camera frustum
		OrthographicCamera(float left, float right, float bottom, float top);

/// @brief Get the position of the camera
/// @return The position of the camera as a constant reference to a glm::vec3 object
		const glm::vec3& GetPosition() const { return m_Position; }
/// @brief Set the position of the camera
/// @param position The new position of the camera as a constant reference to a glm::vec3 object
/// @details This method updates the position of the camera and recalculates the view matrix
		void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateViewMatrix(); }

/// @brief Get the rotation of the camera
/// @return The rotation of the camera as a float
		float GetRotation() const { return m_Rotation; }
/// @brief Set the rotation of the camera
/// @param rotation The new rotation of the camera as a float
/// @details This method updates the rotation of the camera and recalculates the view matrix
		void SetRotation(float rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }

/// @brief Get the projection matrix of the camera
/// @return The projection matrix of the camera as a constant reference to a glm::mat4 object
		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
/// @brief Get the view matrix of the camera
/// @return The view matrix of the camera as a constant reference to a glm::mat4 object
		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
/// @brief Get the view projection matrix of the camera
/// @return The view projection matrix of the camera as a constant reference to a glm::mat4 object
		const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }
	private:
/// @brief Recalculate the view matrix of the camera
/// @details This method recalculates the view matrix based on the current position and rotation of the camera
		void RecalculateViewMatrix();
	private:
/// @brief The projection matrix of the camera
		glm::mat4 m_ProjectionMatrix;
/// @brief The view matrix of the camera
		glm::mat4 m_ViewMatrix;
/// @brief The view projection matrix of the camera
		glm::mat4 m_ViewProjectionMatrix;

/// @brief The position of the camera as a glm::vec3 object
		glm::vec3 m_Position;
/// @brief The rotation of the camera as a float with a default value of 0.0f
		float m_Rotation = 0.0f;
	};
}