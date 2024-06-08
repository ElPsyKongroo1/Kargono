#include "kgpch.h"

#include "Kargono/Renderer/EditorCamera.h"
#include "Kargono/Input/InputPolling.h"
#include "Kargono/Core/KeyCodes.h"
#include "Kargono/Core/MouseCodes.h"
#include "Kargono/Core/EngineCore.h"
#include "Kargono/EditorUI/Editor.h"

#include "API/Windowing/GlfwAPI.h"


namespace Kargono {

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
		if (InputPolling::IsKeyPressed(Key::LeftAlt) || InputPolling::IsKeyPressed(Key::RightAlt))
		{
			EditorUI::Editor::SetDisableLeftClick(true);
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
		if (InputPolling::IsKeyPressed(Key::LeftAlt))
		{
			const Math::vec2& mouse{ InputPolling::GetMouseX(), InputPolling::GetMouseY() };
			Math::vec2 delta = (mouse - m_InitialMousePosition) * 0.003f;
			m_InitialMousePosition = mouse;

			if (InputPolling::IsMouseButtonPressed(Mouse::ButtonMiddle))
				MousePan(delta);
			else if (InputPolling::IsMouseButtonPressed(Mouse::ButtonLeft))
				MouseRotate(delta);
			else if (InputPolling::IsMouseButtonPressed(Mouse::ButtonRight))
				MouseZoom(delta.y);

			UpdateView();
		}
		
	}

	void EditorCamera::OnUpdateFreeFly(Timestep ts)
	{
		if (InputPolling::IsKeyPressed(Key::LeftAlt))
		{
			MouseMovement();
			KeyboardMovement(ts);

			UpdateView();
		}
	}

	void EditorCamera::MouseMovement()
	{
		const Math::vec2& mouse{ InputPolling::GetMouseX(), InputPolling::GetMouseY() };
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

	void EditorCamera::OnEvent(Events::Event& e)
	{
		Events::EventDispatcher dispatcher(e);
		dispatcher.Dispatch<Events::MouseScrolledEvent>(KG_BIND_CLASS_FN(EditorCamera::OnMouseScroll));
		dispatcher.Dispatch<Events::KeyPressedEvent>(KG_BIND_CLASS_FN(EditorCamera::OnKeyPressed));
		dispatcher.Dispatch<Events::KeyReleasedEvent>(KG_BIND_CLASS_FN(EditorCamera::OnKeyReleased));
	}

	bool EditorCamera::OnKeyReleased(Events::KeyReleasedEvent& e)
	{
		if (e.GetKeyCode() == Key::LeftAlt)
		{
			EngineCore::GetActiveWindow().SetMouseCursorVisible(true);
			m_MousePaused = true;
		}
		return false;
	}

	bool EditorCamera::OnKeyPressed(Events::KeyPressedEvent& e)
	{
		if (e.GetKeyCode() == Key::LeftAlt)
		{
			EngineCore::GetActiveWindow().SetMouseCursorVisible(false);
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
		if (InputPolling::IsKeyPressed(Key::LeftAlt))
		{
			float delta = e.GetYOffset() * 0.1f;
			MouseZoom(delta);
			UpdateView();
		}
		return false;
	}

	bool EditorCamera::OnMouseScrollFreeFly(Events::MouseScrolledEvent& e)
	{
		if (InputPolling::IsKeyPressed(Key::LeftAlt))
		{
			float delta = e.GetYOffset() * 5.0f;
			MouseKeyboardSpeed(delta);
		}
		return false;
	}

	void EditorCamera::KeyboardMovement(Timestep ts)
	{
		if (InputPolling::IsKeyPressed(Key::W))
		{
			m_FocalPoint += GetForwardDirection() * static_cast<float>(ts) * m_KeyboardSpeed;
		}
		if (InputPolling::IsKeyPressed(Key::S))
		{
			m_FocalPoint -= GetForwardDirection() * static_cast<float>(ts) * m_KeyboardSpeed;
		}
		if (InputPolling::IsKeyPressed(Key::A))
		{
			m_FocalPoint -= GetRightDirection() * static_cast<float>(ts) * m_KeyboardSpeed;
		}
		if (InputPolling::IsKeyPressed(Key::D))
		{
			m_FocalPoint += GetRightDirection() * static_cast<float>(ts) * m_KeyboardSpeed;
		}
		if (InputPolling::IsKeyPressed(Key::Q))
		{
			m_FocalPoint += Math::vec3(0.0f, -1.0f, 0.0f) * static_cast<float>(ts) * m_KeyboardSpeed;
		}
		if (InputPolling::IsKeyPressed(Key::E))
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
