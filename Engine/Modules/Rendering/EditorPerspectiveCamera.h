#pragma once

#include "Kargono/Core/Timestep.h"
#include "Modules/Rendering/Camera.h"
#include "Modules/Events/Event.h"
#include "Modules/Events/MouseEvent.h"
#include "Kargono/Math/Math.h"
#include "Modules/Events/KeyEvent.h"

namespace Kargono::Rendering
{
	class EditorPerspectiveCamera : public Camera
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
		EditorPerspectiveCamera() = default;
		EditorPerspectiveCamera(float fov, float aspectRatio, float nearClip, float farClip);

		//============================================================
		// Update Camera State Functions
		//============================================================
		// On update typically updates the m_ViewProjection Matrix based on input rules *it defines
		// This function currently manages polling events for camera movement with Alt Key
		void OnUpdate(Timestep ts);
		// On Event typically updates the m_ViewProjection Matrix based on input rules *it defines
		// This function currently manages mouse button clicks
		bool OnInputEvent(Events::Event* e);

		bool OnKeyReleased(Events::KeyReleasedEvent& e);
	private:
		// Movement Type Specific Update Functions
		void OnUpdateModelView(Timestep ts);
		void OnUpdateFreeFly(Timestep ts);
		// Movement Type Specific Event Functions
		bool OnMouseScrollModelView(Events::MouseScrolledEvent& e);
		bool OnMouseScrollFreeFly(Events::MouseScrolledEvent& e);
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

		const Math::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		Math::mat4 GetViewProjection() const { return m_Projection * m_ViewMatrix; }

		void SetViewportSize(float width, float height) 
		{
			m_ViewportWidth = width; 
			m_ViewportHeight = height; 
			UpdateProjection();
		}

		//============================================================
		// Camera Orientation Getters/Setters
		//============================================================

		// Direction vectors used to calculate Math::lookup() matrix
		Math::vec3 GetUpDirection() const;
		Math::vec3 GetRightDirection() const;
		Math::vec3 GetForwardDirection() const;
		// Current Camera Position in World Space
		const Math::vec3& GetPosition() const { return m_Position; }
		// Camera Pitch and Yaw for Camera Rotations
		float GetPitch() const { return m_Pitch; }
		float GetYaw() const { return m_Yaw; }
		void SetPitch(float pitch)
		{
			m_Pitch = pitch;
			UpdateView();
		}
		void SetYaw(float yaw)
		{
			m_Yaw = yaw;
			UpdateView();
		}
		// Change Camera Type
		EditorPerspectiveCamera::MovementType& GetMovementType() { return m_MovementType; }
		void SetMovementType(EditorPerspectiveCamera::MovementType movementType)
		{
			m_MovementType = movementType;
		}
		void ToggleMovementType()
		{
			if (m_MovementType == EditorPerspectiveCamera::MovementType::ModelView)
			{
				m_MovementType = EditorPerspectiveCamera::MovementType::FreeFly;
			}
			else
			{
				m_MovementType = EditorPerspectiveCamera::MovementType::ModelView;
			}
		}

		float& GetMovementSpeed() { return m_KeyboardSpeed; }
		float GetMaxMovementSpeed() const { return m_KeyboardMaxSpeed; }
		float GetMinMovementSpeed() const { return m_KeyboardMinSpeed; }
		void SetFocalPoint(const Math::vec3& focalPoint)
		{
			m_FocalPoint = focalPoint;
			UpdateView();
		}
		Math::vec3 GetFocalPoint()
		{
			return m_FocalPoint;
		}

	private:
		//============================================================
		// Internal Functions
		//============================================================
		Math::quat GetOrientation() const;
		void UpdateProjection();
		void UpdateView();

		bool OnMouseScroll(Events::MouseScrolledEvent& e);
		bool OnKeyPressed(Events::KeyPressedEvent& e);

		void KeyboardMovement(Timestep ts);
		void MouseMovement();

		void MousePan(const Math::vec2& delta);
		void MouseRotate(const Math::vec2& delta);
		void MouseZoom(float delta);
		void MouseKeyboardSpeed(float delta);

		Math::vec3 CalculatePosition() const;

		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;
	private:
		// Camera Movement Type
		EditorPerspectiveCamera::MovementType m_MovementType = MovementType::FreeFly;

		// Camera Projection Fields
		float m_FOV = 45.0f, m_AspectRatio = 1.778f, m_NearClip = 0.1f, m_FarClip = 1000.0f;
		Math::mat4 m_ViewMatrix;
		float m_ViewportWidth = 1280, m_ViewportHeight = 720;

		// Camera Orientation Fields
		Math::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
		Math::vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };
		Math::vec2 m_InitialMousePosition = { 0.0f, 0.0f };
		float m_Distance = 10.0f;
		float m_Pitch = 0.0f, m_Yaw = 0.0f;

		// FreeFly Camera Fields
		float m_KeyboardSpeed = 7.0f;
		float m_KeyboardMinSpeed = 5.0f;
		float m_KeyboardMaxSpeed = 300.0f;
		bool m_MousePaused = true;

	};

}
