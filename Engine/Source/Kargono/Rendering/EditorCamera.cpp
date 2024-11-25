#include "kgpch.h"

#include "Kargono/Rendering/EditorCamera.h"
#include "Kargono/Input/InputService.h"
#include "Kargono/Core/KeyCodes.h"
#include "Kargono/Core/MouseCodes.h"
#include "Kargono/Core/Engine.h"
#include "Kargono/EditorUI/EditorUI.h"

#include "API/Platform/GlfwAPI.h"


namespace Kargono::Rendering
{

	EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip)
		: m_FOV(fov), m_AspectRatio(aspectRatio), m_NearClip(nearClip), m_FarClip(farClip), Camera(glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip))
	{
		UpdateView();
	}

	void EditorCamera::UpdateProjection()
	{
		m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
		m_Projection = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
	}

	void EditorCamera::UpdateView()
	{
		// m_Yaw = m_Pitch = 0.0f; // Lock the camera's rotation
		m_Position = CalculatePosition();

		Math::quat orientation = GetOrientation();
		m_ViewMatrix = glm::translate(Math::mat4(1.0f), m_Position) * glm::toMat4(orientation);
		m_ViewMatrix = glm::inverse(m_ViewMatrix);
	}

	std::pair<float, float> EditorCamera::PanSpeed() const
	{
		float x = std::min(m_ViewportWidth / 1000.0f, 2.4f); // max = 2.4f
		float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		float y = std::min(m_ViewportHeight / 1000.0f, 2.4f); // max = 2.4f
		float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	float EditorCamera::RotationSpeed() const
	{
		return 0.8f;
	}

	float EditorCamera::ZoomSpeed() const
	{
		float distance = m_Distance * 0.2f;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;
		speed = std::min(speed, 100.0f); // max speed = 100
		return speed;
	}

	void EditorCamera::OnUpdate(Timestep ts)
	{
		if (Input::InputService::IsKeyPressed(Key::LeftAlt) || Input::InputService::IsKeyPressed(Key::RightAlt))
		{
			EditorUI::EditorUIService::SetDisableLeftClick(true);
		}

		switch (m_MovementType)
		{
		case EditorCamera::MovementType::ModelView:
			OnUpdateModelView(ts);
			return;
		case EditorCamera::MovementType::FreeFly:
			OnUpdateFreeFly(ts);
			return;
		case EditorCamera::MovementType::None:
			return;
		}

		KG_ERROR("Invalid Enum Type for m_MovementType");
	}

	void EditorCamera::OnUpdateModelView(Timestep ts)
	{
		if (Input::InputService::IsKeyPressed(Key::LeftAlt))
		{
			const Math::vec2& mouse{ Input::InputService::GetMouseX(), Input::InputService::GetMouseY() };
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

	void EditorCamera::OnUpdateFreeFly(Timestep ts)
	{
		if (Input::InputService::IsKeyPressed(Key::LeftAlt))
		{
			MouseMovement();
			KeyboardMovement(ts);

			UpdateView();
		}
	}

	void EditorCamera::MouseMovement()
	{
		const Math::vec2& mouse{ Input::InputService::GetMouseX(), Input::InputService::GetMouseY() };
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

	bool EditorCamera::OnInputEvent(Events::Event* e)
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

	bool EditorCamera::OnKeyReleased(Events::KeyReleasedEvent& e)
	{
		if (e.GetKeyCode() == Key::LeftAlt)
		{
			EngineService::GetActiveWindow().SetMouseCursorVisible(true);
			m_MousePaused = true;
		}
		return false;
	}

	bool EditorCamera::OnKeyPressed(Events::KeyPressedEvent& e)
	{
		if (e.GetKeyCode() == Key::LeftAlt)
		{
			EngineService::GetActiveWindow().SetMouseCursorVisible(false);
			m_MousePaused = true;
		}
		return false;
	}

	bool EditorCamera::OnMouseScroll(Events::MouseScrolledEvent& e)
	{
		switch (m_MovementType)
		{
		case EditorCamera::MovementType::ModelView:
			return OnMouseScrollModelView(e);
		case EditorCamera::MovementType::FreeFly:
			return OnMouseScrollFreeFly(e);
		case EditorCamera::MovementType::None:
			return false;
		}

		KG_ERROR("Invalid Enum Type for m_MovementType");
		return false;
	}

	bool EditorCamera::OnMouseScrollModelView(Events::MouseScrolledEvent& e)
	{
		if (Input::InputService::IsKeyPressed(Key::LeftAlt))
		{
			float delta = e.GetYOffset() * 0.1f;
			MouseZoom(delta);
			UpdateView();
		}
		return false;
	}

	bool EditorCamera::OnMouseScrollFreeFly(Events::MouseScrolledEvent& e)
	{
		if (Input::InputService::IsKeyPressed(Key::LeftAlt))
		{
			float delta = e.GetYOffset() * 5.0f;
			MouseKeyboardSpeed(delta);
		}
		return false;
	}

	void EditorCamera::KeyboardMovement(Timestep ts)
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

	void EditorCamera::MousePan(const Math::vec2& delta)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		m_FocalPoint += -GetRightDirection() * delta.x * xSpeed * m_Distance;
		m_FocalPoint += GetUpDirection() * delta.y * ySpeed * m_Distance;
	}

	void EditorCamera::MouseRotate(const Math::vec2& delta)
	{
		float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		m_Yaw += yawSign * delta.x * RotationSpeed();
		m_Pitch += delta.y * RotationSpeed();
	}

	void EditorCamera::MouseZoom(float delta)
	{
		m_Distance -= delta * ZoomSpeed();
		if (m_Distance < 1.0f)
		{
			m_FocalPoint += GetForwardDirection();
			m_Distance = 1.0f;
		}
	}

	void EditorCamera::MouseKeyboardSpeed(float delta)
	{
		if (delta + m_KeyboardSpeed > m_KeyboardMaxSpeed) { m_KeyboardSpeed = m_KeyboardMaxSpeed; return; }
		if (delta + m_KeyboardSpeed < m_KeyboardMinSpeed) { m_KeyboardSpeed = m_KeyboardMinSpeed; return; }
		m_KeyboardSpeed += delta;
	}

	Math::vec3 EditorCamera::GetUpDirection() const
	{
		return glm::rotate(GetOrientation(), Math::vec3(0.0f, 1.0f, 0.0f));
	}

	Math::vec3 EditorCamera::GetRightDirection() const
	{
		return glm::rotate(GetOrientation(), Math::vec3(1.0f, 0.0f, 0.0f));
	}

	Math::vec3 EditorCamera::GetForwardDirection() const
	{
		return glm::rotate(GetOrientation(), Math::vec3(0.0f, 0.0f, -1.0f));
	}

	Math::vec3 EditorCamera::CalculatePosition() const
	{
		return m_FocalPoint - GetForwardDirection() * m_Distance;
	}

	Math::quat EditorCamera::GetOrientation() const
	{
		return Math::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
	}

}
