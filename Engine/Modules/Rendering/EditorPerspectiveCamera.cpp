#include "kgpch.h"

#include "Modules/Rendering/EditorPerspectiveCamera.h"
#include "Modules/Input/InputService.h"
#include "Kargono/Core/KeyCodes.h"
#include "Kargono/Core/MouseCodes.h"
#include "Modules/Core/Engine.h"
#include "Modules/EditorUI/EditorUI.h"

#include "API/Platform/GlfwAPI.h"


namespace Kargono::Rendering
{

	EditorPerspectiveCamera::EditorPerspectiveCamera(float fov, float aspectRatio, float nearClip, float farClip)
		: m_FOV(fov), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip), Camera(glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip))
	{
		UpdateView();
	}

	void EditorPerspectiveCamera::UpdateProjection()
	{
		m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
		m_Projection = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
	}

	void EditorPerspectiveCamera::UpdateView()
	{
		// m_Yaw = m_Pitch = 0.0f; // Lock the camera's rotation
		m_Position = CalculatePosition();

		Math::quat orientation = GetOrientation();
		m_ViewMatrix = glm::translate(Math::mat4(1.0f), m_Position) * glm::toMat4(orientation);
		m_ViewMatrix = glm::inverse(m_ViewMatrix);
	}

	std::pair<float, float> EditorPerspectiveCamera::PanSpeed() const
	{
		float x = std::min(m_ViewportWidth / 1000.0f, 2.4f); // max = 2.4f
		float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		float y = std::min(m_ViewportHeight / 1000.0f, 2.4f); // max = 2.4f
		float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	float EditorPerspectiveCamera::RotationSpeed() const
	{
		return 0.8f;
	}

	float EditorPerspectiveCamera::ZoomSpeed() const
	{
		float distance = m_Distance * 0.2f;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;
		speed = std::min(speed, 100.0f); // max speed = 100
		return speed;
	}

	void EditorPerspectiveCamera::OnUpdate(Timestep ts)
	{
		if (Input::InputService::IsKeyPressed(Key::LeftAlt) || Input::InputService::IsKeyPressed(Key::RightAlt))
		{
			EditorUI::EditorUIService::SetDisableLeftClick(true);
		}

		switch (m_MovementType)
		{
		case EditorPerspectiveCamera::MovementType::ModelView:
			OnUpdateModelView(ts);
			return;
		case EditorPerspectiveCamera::MovementType::FreeFly:
			OnUpdateFreeFly(ts);
			return;
		case EditorPerspectiveCamera::MovementType::None:
			return;
		}

		KG_ERROR("Invalid Enum Type for m_MovementType");
	}

	void EditorPerspectiveCamera::OnUpdateModelView(Timestep ts)
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

	void EditorPerspectiveCamera::OnUpdateFreeFly(Timestep ts)
	{
		if (Input::InputService::IsKeyPressed(Key::LeftAlt))
		{
			MouseMovement();
			KeyboardMovement(ts);

			UpdateView();
		}
	}

	void EditorPerspectiveCamera::MouseMovement()
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

	bool EditorPerspectiveCamera::OnInputEvent(Events::Event* e)
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

	bool EditorPerspectiveCamera::OnKeyReleased(Events::KeyReleasedEvent& e)
	{
		if (e.GetKeyCode() == Key::LeftAlt)
		{
			EngineService::GetActiveEngine().GetWindow().SetMouseCursorVisible(true);
			m_MousePaused = true;
		}
		return false;
	}

	bool EditorPerspectiveCamera::OnKeyPressed(Events::KeyPressedEvent& e)
	{
		if (e.GetKeyCode() == Key::LeftAlt)
		{
			EngineService::GetActiveEngine().GetWindow().SetMouseCursorVisible(false);
			m_MousePaused = true;
		}
		return false;
	}

	bool EditorPerspectiveCamera::OnMouseScroll(Events::MouseScrolledEvent& e)
	{
		switch (m_MovementType)
		{
		case EditorPerspectiveCamera::MovementType::ModelView:
			return OnMouseScrollModelView(e);
		case EditorPerspectiveCamera::MovementType::FreeFly:
			return OnMouseScrollFreeFly(e);
		case EditorPerspectiveCamera::MovementType::None:
			return false;
		}

		KG_ERROR("Invalid Enum Type for m_MovementType");
		return false;
	}

	bool EditorPerspectiveCamera::OnMouseScrollModelView(Events::MouseScrolledEvent& e)
	{
		if (Input::InputService::IsKeyPressed(Key::LeftAlt))
		{
			float delta = e.GetYOffset() * 0.1f;
			MouseZoom(delta);
			UpdateView();
		}
		return false;
	}

	bool EditorPerspectiveCamera::OnMouseScrollFreeFly(Events::MouseScrolledEvent& e)
	{
		if (Input::InputService::IsKeyPressed(Key::LeftAlt))
		{
			float delta = e.GetYOffset() * 5.0f;
			MouseKeyboardSpeed(delta);
		}
		return false;
	}

	void EditorPerspectiveCamera::KeyboardMovement(Timestep ts)
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

	void EditorPerspectiveCamera::MousePan(const Math::vec2& delta)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		m_FocalPoint += -GetRightDirection() * delta.x * xSpeed * m_Distance;
		m_FocalPoint += GetUpDirection() * delta.y * ySpeed * m_Distance;
	}

	void EditorPerspectiveCamera::MouseRotate(const Math::vec2& delta)
	{
		float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		m_Yaw += yawSign * delta.x * RotationSpeed();
		m_Pitch += delta.y * RotationSpeed();
	}

	void EditorPerspectiveCamera::MouseZoom(float delta)
	{
		m_Distance -= delta * ZoomSpeed();
		if (m_Distance < 1.0f)
		{
			m_FocalPoint += GetForwardDirection();
			m_Distance = 1.0f;
		}
	}

	void EditorPerspectiveCamera::MouseKeyboardSpeed(float delta)
	{
		if (delta + m_KeyboardSpeed > m_KeyboardMaxSpeed) { m_KeyboardSpeed = m_KeyboardMaxSpeed; return; }
		if (delta + m_KeyboardSpeed < m_KeyboardMinSpeed) { m_KeyboardSpeed = m_KeyboardMinSpeed; return; }
		m_KeyboardSpeed += delta;
	}

	Math::vec3 EditorPerspectiveCamera::GetUpDirection() const
	{
		return glm::rotate(GetOrientation(), Math::vec3(0.0f, 1.0f, 0.0f));
	}

	Math::vec3 EditorPerspectiveCamera::GetRightDirection() const
	{
		return glm::rotate(GetOrientation(), Math::vec3(1.0f, 0.0f, 0.0f));
	}

	Math::vec3 EditorPerspectiveCamera::GetForwardDirection() const
	{
		return glm::rotate(GetOrientation(), Math::vec3(0.0f, 0.0f, -1.0f));
	}

	Math::vec3 EditorPerspectiveCamera::CalculatePosition() const
	{
		return m_FocalPoint - GetForwardDirection() * m_Distance;
	}

	Math::quat EditorPerspectiveCamera::GetOrientation() const
	{
		return Math::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
	}

}
