#pragma once

#include "Kargono/Core/Timestep.h"
#include "Modules/Cameras/CameraProjection.h"
#include "Modules/Events/Event.h"
#include "Modules/Events/MouseEvent.h"
#include "Kargono/Math/Math.h"
#include "Modules/Events/KeyEvent.h"

namespace Kargono::Cameras
{
	enum MovementType
	{
		None = 0, 
		ModelView, 
		FreeFly
	};

	class PerspectiveCamera
	{
	public:
		//==============================
		// Constructors/Destructors
		//==============================
		PerspectiveCamera() = default;
		PerspectiveCamera(float fov, float aspectRatio, float nearClip, float farClip);
	public:
		//==============================
		// OnEvent
		//==============================
		void OnUpdate(Timestep ts);
		bool OnInputEvent(Events::Event* e);
		bool OnKeyReleased(Events::KeyReleasedEvent& e);
	private:
		// Internal event handlers
		bool OnMouseScroll(Events::MouseScrolledEvent& e);
		bool OnKeyPressed(Events::KeyPressedEvent& e);
		// Helpers
		void OnUpdateModelView(Timestep ts);
		void OnUpdateFreeFly(Timestep ts);
		bool OnMouseScrollModelView(Events::MouseScrolledEvent& e);
		bool OnMouseScrollFreeFly(Events::MouseScrolledEvent& e);
	public:
		//==============================
		// Getters/Setters
		//==============================
		float GetDistance() const { return m_Distance; }
		void SetDistance(float distance) { m_Distance = distance; UpdateView(); }
		const Math::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		Math::mat4 GetViewProjection() const { return m_CameraProjection.GetProjection() * m_ViewMatrix; }
		void SetViewportSize(float width, float height) 
		{
			m_ViewportWidth = width; 
			m_ViewportHeight = height; 
			UpdateProjection();
		}
		Math::vec3 GetUpDirection() const;
		Math::vec3 GetRightDirection() const;
		Math::vec3 GetForwardDirection() const;
		const Math::vec3& GetPosition() const { return m_Position; }
		float GetPitch() const { return m_Pitch; }
		float GetYaw() const { return m_Yaw; }
		void SetPitch(float pitch) { m_Pitch = pitch; UpdateView(); }
		void SetYaw(float yaw) { m_Yaw = yaw; UpdateView(); }
		MovementType& GetMovementType() { return m_MovementType; }
		void SetMovementType(MovementType movementType) { m_MovementType = movementType; }
		void ToggleMovementType();
		float& GetMovementSpeed() { return m_KeyboardSpeed; }
		float GetMaxMovementSpeed() const { return m_KeyboardMaxSpeed; }
		float GetMinMovementSpeed() const { return m_KeyboardMinSpeed; }
		void SetFocalPoint(const Math::vec3& focalPoint) { m_FocalPoint = focalPoint; UpdateView(); }
		Math::vec3 GetFocalPoint() { return m_FocalPoint; }
	private:
		// Helper(s)
		Math::quat GetOrientation() const;
	private:
		//==============================
		// Revalidate Matrices
		//==============================
		void UpdateProjection();
		void UpdateView();
		Math::vec3 CalculatePosition() const;
	private:
		//==============================
		// Handle Movement
		//==============================
		void KeyboardMovement(Timestep ts);
		void MouseMovement();
		void MousePan(const Math::vec2& delta);
		void MouseRotate(const Math::vec2& delta);
		void MouseZoom(float delta);
		void MouseKeyboardSpeed(float delta);
	private:
		//==============================
		// Manage Speed
		//==============================
		std::pair<float, float> PanSpeed() const;
		float RotationSpeed() const;
		float ZoomSpeed() const;
	private:
		// Camera Movement Type
		MovementType m_MovementType{ MovementType::FreeFly };
		// Camera Projection Fields
		CameraProjection m_CameraProjection{};
		float m_FOV{ 45.0f };
		float m_AspectRatio{ 1.778f };
		float m_NearClip{ 0.1f };
		float m_FarClip{ 1000.0f };
		Math::mat4 m_ViewMatrix;
		float m_ViewportWidth{ 1280 };
		float m_ViewportHeight{ 720 };
		// Camera Orientation Fields
		Math::vec3 m_Position { 0.0f, 0.0f, 0.0f };
		Math::vec3 m_FocalPoint { 0.0f, 0.0f, 0.0f };
		Math::vec2 m_InitialMousePosition { 0.0f, 0.0f };
		float m_Distance{ 10.0f };
		float m_Pitch{ 0.0f };
		float m_Yaw{ 0.0f };
		// FreeFly Camera Fields
		float m_KeyboardSpeed{ 7.0f };
		float m_KeyboardMinSpeed{ 5.0f };
		float m_KeyboardMaxSpeed{ 300.0f };
		bool m_MousePaused{ true };
	};

}
