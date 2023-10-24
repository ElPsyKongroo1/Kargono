#pragma once

#include "Kargono/Core/Timestep.h"
#include "Kargono/Renderer/Camera.h"
#include "Kargono/Events/Event.h"
#include "Kargono/Events/MouseEvent.h"

#include <glm/glm.hpp>

#include "Kargono/Events/KeyEvent.h"

namespace Kargono
{
	class EditorCamera : public Camera
	{

	public:
		// This enum determines the Camera's current movement functionality
		enum MovementType
		{
			None = 0, ModelView, FreeFly
		};

	
		//============================================================
		// Camera Constructors
		//============================================================
		// This camera is only meant to be instantiated in the editor
		EditorCamera() = default;
		EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

		//============================================================
		// Update Camera State Functions
		//============================================================
		// On update typically updates the m_ViewProjection Matrix based on input rules *it defines
		// This function currently manages polling events for camera movement with Alt Key
		void OnUpdate(Timestep ts);
		// On Event typically updates the m_ViewProjection Matrix based on input rules *it defines
		// This function currently manages mouse button clicks
		void OnEvent(Event& e);
	private:
		// Movement Type Specific Update Functions
		void OnUpdateModelView(Timestep ts);
		void OnUpdateFreeFly(Timestep ts);
		// Movement Type Specific Event Functions
		bool OnMouseScrollModelView(MouseScrolledEvent& e);
		bool OnMouseScrollFreeFly(MouseScrolledEvent& e);
	public:

		//============================================================
		// ModelView Movement Getter/Setters
		//============================================================
		float GetDistance() const { return m_Distance; }
		void SetDistance(float distance)
		{
			m_Distance = distance;
			UpdateView();
		}

		//============================================================
		// View/Projection Matrix Getter/Setter
		//============================================================

		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		glm::mat4 GetViewProjection() const { return m_Projection * m_ViewMatrix; }

		void SetViewportSize(float width, float height) { m_ViewportWidth = width; m_ViewportHeight = height; UpdateProjection(); }

		//============================================================
		// Camera Orientation Getters/Setters
		//============================================================

		// Direction vectors used to calculate glm::lookup() matrix
		glm::vec3 GetUpDirection() const;
		glm::vec3 GetRightDirection() const;
		glm::vec3 GetForwardDirection() const;
		// Current Camera Position in World Space
		const glm::vec3& GetPosition() const { return m_Position; }
		// Camera Pitch and Yaw for Camera Rotations
		float GetPitch() const { return m_Pitch; }
		float GetYaw() const { return m_Yaw; }
		// Change Camera Type
		EditorCamera::MovementType& GetMovementType() { return m_MovementType; }
		void SetMovementType(EditorCamera::MovementType movementType)
		{
			m_MovementType = movementType;
		}
		void ToggleMovementType()
		{
			if (m_MovementType == EditorCamera::MovementType::ModelView)
			{
				m_MovementType = EditorCamera::MovementType::FreeFly;
			}
			else
			{
				m_MovementType = EditorCamera::MovementType::ModelView;
			}
		}

		float& GetMovementSpeed() { return m_KeyboardSpeed; }
		float GetMaxMovementSpeed() const { return m_KeyboardMaxSpeed; }
		float GetMinMovementSpeed() const { return m_KeyboardMinSpeed; }
		void SetFocalPoint(const glm::vec3& focalPoint)
		{
			m_FocalPoint = focalPoint;
			UpdateView();
		}

	private:
		//============================================================
		// Internal Functions
		//============================================================
		glm::quat GetOrientation() const;
		void UpdateProjection();
		void UpdateView();

		bool OnMouseScroll(MouseScrolledEvent& e);
		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnKeyReleased(KeyReleasedEvent& e);

		void KeyboardMovement(Timestep ts);
		void MouseMovement();

		void MousePan(const glm::vec2& delta);
		void MouseRotate(const glm::vec2& delta);
		void MouseZoom(float delta);
		void MouseKeyboardSpeed(float delta);

		glm::vec3 CalculatePosition() const;

		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;
	private:
		// Camera Movement Type
		EditorCamera::MovementType m_MovementType = MovementType::ModelView;

		// Camera Projection Fields
		float m_FOV = 45.0f, m_AspectRatio = 1.778f, m_NearClip = 0.1f, m_FarClip = 1000.0f;
		glm::mat4 m_ViewMatrix;
		float m_ViewportWidth = 1280, m_ViewportHeight = 720;

		// Camera Orientation Fields
		glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };
		glm::vec2 m_InitialMousePosition = { 0.0f, 0.0f };
		float m_Distance = 10.0f;
		float m_Pitch = 0.0f, m_Yaw = 0.0f;

		// FreeFly Camera Fields
		float m_KeyboardSpeed = 7.0f;
		float m_KeyboardMinSpeed = 0.0f;
		float m_KeyboardMaxSpeed = 300.0f;
		bool m_MousePaused = true;

	};

}
