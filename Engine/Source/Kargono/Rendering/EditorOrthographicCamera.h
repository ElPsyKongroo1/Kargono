#pragma once

#include "Kargono/Core/Timestep.h"
#include "Kargono/Rendering/Camera.h"
#include "Kargono/Events/Event.h"
#include "Kargono/Events/MouseEvent.h"
#include "Kargono/Math/Math.h"
#include "Kargono/Events/KeyEvent.h"

namespace Kargono::Rendering
{
	class EditorOrthographicCamera : public Rendering::Camera
	{
	public:
		EditorOrthographicCamera();
		EditorOrthographicCamera(float size, float nearClip, float farClip);
		virtual ~EditorOrthographicCamera() = default;

	public:
		void OnUpdate(Timestep ts);
		bool OnKeyReleased(Events::KeyReleasedEvent& e);
		bool OnKeyPressed(Events::KeyPressedEvent& e);
		bool OnInputEvent(Events::Event* e);
		bool OnMouseScroll(Events::MouseScrolledEvent& e);
		void OnViewportResize();

	public:
		float GetOrthographicSize() const
		{
			return m_CameraSize;
		}
		void SetOrthographicSize(float size)
		{
			m_CameraSize = size;
			RecalculateProjection();
		}
		float GetOrthographicNearClip() const
		{
			return m_NearPlane;
		}
		void SetOrthographicNearClip(float nearClip)
		{
			m_NearPlane = nearClip;
			RecalculateProjection();
		}
		float GetOrthographicFarClip() const
		{
			return m_FarPlane;
		}
		void SetOrthographicFarClip(float farClip)
		{
			m_FarPlane = farClip;
			RecalculateProjection();
		}
		void SetPosition(const Math::vec3& position)
		{
			m_Position = position;
			RecalculateView();
		}
		const Math::vec3& GetPosition() const 
		{ 
			return m_Position; 
		}
		void SetOrientation(const Math::vec3& orientation)
		{
			m_Orientation = orientation;
			RecalculateView();
		}
		const Math::vec3& GetOrientation() const
		{
			return m_Orientation;
		}
		
		void SetKeyboardSpeed(float speed)
		{
			m_KeyboardSpeed = speed;
		}
		float GetKeyboardSpeed() const
		{
			return m_KeyboardSpeed;
		}

		const Math::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		Math::mat4 GetViewProjection() const { return m_Projection * m_ViewMatrix; }
	private:
		void RecalculateProjection();
		void RecalculateView();
	private:
		float m_CameraSize{ 10.0f };
		float m_NearPlane{ 1.0f };
		float m_FarPlane{ 10.0f };

		Math::mat4 m_ViewMatrix;
		Math::vec3 m_Position{ 0.0f, 0.0f, 0.0f };
		Math::vec3 m_Orientation{ 0.0f, 0.0f, 0.0f };

		float m_KeyboardSpeed{ 7.0f };
		float m_KeyboardMinSpeed{ 5.0f };
		float m_KeyboardMaxSpeed{ 300.0f };
	};

}
