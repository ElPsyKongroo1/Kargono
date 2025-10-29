#include "kgpch.h"

#include "Modules/Cameras/PerspectiveCamera.h"
#include "Modules/Input/InputService.h"
#include "Kargono/Core/KeyCodes.h"
#include "Kargono/Core/MouseCodes.h"
#include "Modules/Core/Engine.h"
#include "Modules/EditorUI/EditorUIInclude.h"

#include "API/Platform/GlfwAPI.h"


namespace Kargono::Cameras
{
	PerspectiveCamera::PerspectiveCamera(float fov, float aspectRatio, float nearClip, float farClip)
		: m_FOV(fov), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip), 
		m_CameraProjection(glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip))
	{
		UpdateView();
	}

	void PerspectiveCamera::UpdateProjection()
	{
		m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
		m_CameraProjection.SetProjection(glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip));
	}

	void PerspectiveCamera::UpdateView()
	{
		// m_Yaw = m_Pitch = 0.0f; // Lock the camera's rotation
		m_Position = CalculatePosition();

		Math::quat orientation = GetOrientation();
		m_ViewMatrix = glm::translate(Math::mat4(1.0f), m_Position) * glm::toMat4(orientation);
		m_ViewMatrix = glm::inverse(m_ViewMatrix);
	}

	std::pair<float, float> PerspectiveCamera::PanSpeed() const
	{
		float x = std::min(m_ViewportWidth / 1000.0f, 2.4f); // max = 2.4f
		float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		float y = std::min(m_ViewportHeight / 1000.0f, 2.4f); // max = 2.4f
		float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	float PerspectiveCamera::RotationSpeed() const
	{
		return 0.8f;
	}

	float PerspectiveCamera::ZoomSpeed() const
	{
		float distance = m_Distance * 0.2f;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;
		speed = std::min(speed, 100.0f); // max speed = 100
		return speed;
	}

	void PerspectiveCamera::OnUpdate(Timestep ts)
	{
		if (Input::InputService::IsKeyPressed(Key::LeftAlt) || Input::InputService::IsKeyPressed(Key::RightAlt))
		{
			EditorUI::EditorUIContext::SetDisableLeftClick(true);
		}

		switch (m_MovementType)
		{
		case MovementType::ModelView:
			OnUpdateModelView(ts);
			return;
		case MovementType::FreeFly:
			OnUpdateFreeFly(ts);
			return;
		case MovementType::None:
			return;
		}

		KG_ERROR("Invalid Enum Type for m_MovementType");
	}

	void PerspectiveCamera::OnUpdateModelView(Timestep ts)
	{
		UNREFERENCED_PARAMETER(ts);

		if (Input::InputService::IsKeyPressed(Key::LeftAlt))
		{
			const Math::vec2& mouse{ Input::InputService::GetAbsoluteMouseX(), Input::InputService::GetAbsoluteMouseY() };
			Math::vec2 delta = (mouse - m_InitialMousePosition) * 0.003f;
			m_InitialMousePosition = mouse;

			if (Input::InputService::IsMouseButtonPressed(Mouse::ButtonMiddle))
				MousePan(delta);
			else if (Input::InputService::IsMouseButtonPressed(Mouse::ButtonLeft))
				MouseRotate(delta);
			else if (Input::InputService::IsMouseButtonPressed(Mouse::ButtonRight))
				MouseZoom(delta.y);

			UpdateView();
		}
		
	}

	void PerspectiveCamera::OnUpdateFreeFly(Timestep ts)
	{
		if (Input::InputService::IsKeyPressed(Key::LeftAlt))
		{
			MouseMovement();
			KeyboardMovement(ts);

			UpdateView();
		}
	}

	void PerspectiveCamera::MouseMovement()
	{
		const Math::vec2& mouse{ Input::InputService::GetAbsoluteMouseX(), Input::InputService::GetAbsoluteMouseY() };
		Math::vec2 delta = (mouse - m_InitialMousePosition) * 0.003f;
		m_InitialMousePosition = mouse;
		if (m_MousePaused)
		{
			m_MousePaused = false;
			delta = { 0.0f, 0.0f };
		}
		// Adjust Yaw and Pitch Values
		m_Yaw += delta.x;
		m_Pitch += delta.y;
		m_FocalPoint = m_Position + GetForwardDirection() * m_Distance;
	}

	bool PerspectiveCamera::OnInputEvent(Events::Event* e)
	{
		bool handled = false;
		switch (e->GetEventType())
		{
		case Events::EventType::MouseScrolled:
			handled = OnMouseScroll(*(Events::MouseScrolledEvent*)e);
			break;
		case Events::EventType::KeyPressed:
			handled = OnKeyPressed(*(Events::KeyPressedEvent*)e);
			break;
		case Events::EventType::KeyReleased:
			handled = OnKeyReleased(*(Events::KeyReleasedEvent*)e);
			break;
		}

		if (handled)
		{
			return true;
		}

		return false;
	}

	bool PerspectiveCamera::OnKeyReleased(Events::KeyReleasedEvent& e)
	{
		if (e.GetKeyCode() == Key::LeftAlt)
		{
			EngineService::GetActiveEngine().GetWindow().SetMouseCursorVisible(true);
			m_MousePaused = true;
		}
		return false;
	}

	bool PerspectiveCamera::OnKeyPressed(Events::KeyPressedEvent& e)
	{
		if (e.GetKeyCode() == Key::LeftAlt)
		{
			EngineService::GetActiveEngine().GetWindow().SetMouseCursorVisible(false);
			m_MousePaused = true;
		}
		return false;
	}

	bool PerspectiveCamera::OnMouseScroll(Events::MouseScrolledEvent& e)
	{
		switch (m_MovementType)
		{
		case MovementType::ModelView:
			return OnMouseScrollModelView(e);
		case MovementType::FreeFly:
			return OnMouseScrollFreeFly(e);
		case MovementType::None:
			return false;
		}

		KG_ERROR("Invalid Enum Type for m_MovementType");
		return false;
	}

	bool PerspectiveCamera::OnMouseScrollModelView(Events::MouseScrolledEvent& e)
	{
		if (Input::InputService::IsKeyPressed(Key::LeftAlt))
		{
			float delta = e.GetYOffset() * 0.1f;
			MouseZoom(delta);
			UpdateView();
		}
		return false;
	}

	bool PerspectiveCamera::OnMouseScrollFreeFly(Events::MouseScrolledEvent& e)
	{
		if (Input::InputService::IsKeyPressed(Key::LeftAlt))
		{
			float delta = e.GetYOffset() * 5.0f;
			MouseKeyboardSpeed(delta);
		}
		return false;
	}

	void PerspectiveCamera::KeyboardMovement(Timestep ts)
	{
		if (Input::InputService::IsKeyPressed(Key::W))
		{
			m_FocalPoint += GetForwardDirection() * static_cast<float>(ts) * m_KeyboardSpeed;
		}
		if (Input::InputService::IsKeyPressed(Key::S))
		{
			m_FocalPoint -= GetForwardDirection() * static_cast<float>(ts) * m_KeyboardSpeed;
		}
		if (Input::InputService::IsKeyPressed(Key::A))
		{
			m_FocalPoint -= GetRightDirection() * static_cast<float>(ts) * m_KeyboardSpeed;
		}
		if (Input::InputService::IsKeyPressed(Key::D))
		{
			m_FocalPoint += GetRightDirection() * static_cast<float>(ts) * m_KeyboardSpeed;
		}
		if (Input::InputService::IsKeyPressed(Key::Q))
		{
			m_FocalPoint += Math::vec3(0.0f, -1.0f, 0.0f) * static_cast<float>(ts) * m_KeyboardSpeed;
		}
		if (Input::InputService::IsKeyPressed(Key::E))
		{
			m_FocalPoint += Math::vec3(0.0f, 1.0f, 0.0f) * static_cast<float>(ts) * m_KeyboardSpeed;
		}
	}

	void PerspectiveCamera::ToggleMovementType()
	{
		if (m_MovementType == MovementType::ModelView)
		{
			m_MovementType = MovementType::FreeFly;
		}
		else
		{
			m_MovementType = MovementType::ModelView;
		}
	}

	void PerspectiveCamera::MousePan(const Math::vec2& delta)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		m_FocalPoint += -GetRightDirection() * delta.x * xSpeed * m_Distance;
		m_FocalPoint += GetUpDirection() * delta.y * ySpeed * m_Distance;
	}

	void PerspectiveCamera::MouseRotate(const Math::vec2& delta)
	{
		float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		m_Yaw += yawSign * delta.x * RotationSpeed();
		m_Pitch += delta.y * RotationSpeed();
	}

	void PerspectiveCamera::MouseZoom(float delta)
	{
		m_Distance -= delta * ZoomSpeed();
		if (m_Distance < 1.0f)
		{
			m_FocalPoint += GetForwardDirection();
			m_Distance = 1.0f;
		}
	}

	void PerspectiveCamera::MouseKeyboardSpeed(float delta)
	{
		if (delta + m_KeyboardSpeed > m_KeyboardMaxSpeed) { m_KeyboardSpeed = m_KeyboardMaxSpeed; return; }
		if (delta + m_KeyboardSpeed < m_KeyboardMinSpeed) { m_KeyboardSpeed = m_KeyboardMinSpeed; return; }
		m_KeyboardSpeed += delta;
	}

	Math::vec3 PerspectiveCamera::GetUpDirection() const
	{
		return glm::rotate(GetOrientation(), Math::vec3(0.0f, 1.0f, 0.0f));
	}

	Math::vec3 PerspectiveCamera::GetRightDirection() const
	{
		return glm::rotate(GetOrientation(), Math::vec3(1.0f, 0.0f, 0.0f));
	}

	Math::vec3 PerspectiveCamera::GetForwardDirection() const
	{
		return glm::rotate(GetOrientation(), Math::vec3(0.0f, 0.0f, -1.0f));
	}

	Math::vec3 PerspectiveCamera::CalculatePosition() const
	{
		return m_FocalPoint - GetForwardDirection() * m_Distance;
	}

	Math::quat PerspectiveCamera::GetOrientation() const
	{
		return Math::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
	}

}
