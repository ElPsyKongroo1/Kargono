#include "kgpch.h"

#include "Kargono/Rendering/EditorOrthographicCamera.h"
#include "Kargono/Input/InputService.h"
#include "Kargono/Core/KeyCodes.h"
#include "Kargono/Core/MouseCodes.h"
#include "Kargono/Core/Engine.h"
#include "Kargono/EditorUI/EditorUI.h"

#include "API/Platform/GlfwAPI.h"


namespace Kargono::Rendering
{

	EditorOrthographicCamera::EditorOrthographicCamera(float size, float nearClip, float farClip)
	{
		m_CameraSize = size;
		m_NearPlane = nearClip;
		m_FarPlane = farClip;
		RecalculateProjection();
		RecalculateView();
	}
	EditorOrthographicCamera::EditorOrthographicCamera()
	{
		RecalculateProjection();
		RecalculateView();
	}
	void EditorOrthographicCamera::OnViewportResize()
	{
		RecalculateProjection();
	}

	void EditorOrthographicCamera::OnUpdate(Timestep ts)
	{
		// Move camera with based on keyboard input
		if (!Input::InputService::IsKeyPressed(Key::LeftAlt) && !Input::InputService::IsKeyPressed(Key::RightAlt))
		{
			return;
		}
		bool updated{ false };
		if (Input::InputService::IsKeyPressed(Key::W))
		{
			m_Position.y += m_KeyboardSpeed * ts;
			updated = true;
		}
		if (Input::InputService::IsKeyPressed(Key::S))
		{
			m_Position.y -= m_KeyboardSpeed * ts;
			updated = true;
		}
		if (Input::InputService::IsKeyPressed(Key::D))
		{
			m_Position.x += m_KeyboardSpeed * ts;
			updated = true;
		}
		if (Input::InputService::IsKeyPressed(Key::A))
		{
			m_Position.x -= m_KeyboardSpeed * ts;
			updated = true;
		}

		// Recalculate the view matrix if the camera location has been updated
		if (updated)
		{
			RecalculateView();
		}
	}

	bool EditorOrthographicCamera::OnKeyReleased(Events::KeyReleasedEvent& e)
	{
		if (e.GetKeyCode() == Key::LeftAlt)
		{
			EngineService::GetActiveWindow().SetMouseCursorVisible(true);
		}
		return false;
	}

	bool EditorOrthographicCamera::OnKeyPressed(Events::KeyPressedEvent& e)
	{
		if (e.GetKeyCode() == Key::LeftAlt)
		{
			EngineService::GetActiveWindow().SetMouseCursorVisible(false);
		}
		return false;
	}

	bool EditorOrthographicCamera::OnInputEvent(Events::Event* e)
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

	bool EditorOrthographicCamera::OnMouseScroll(Events::MouseScrolledEvent& e)
	{
		if (Input::InputService::IsKeyPressed(Key::LeftAlt))
		{
			float delta = e.GetYOffset() * 15.0f;
			
			if (m_CameraSize - delta <= 5.0f)
			{
				return false;
			}
			m_CameraSize -= delta;
			RecalculateProjection();
		}

		return false;
	}
	void EditorOrthographicCamera::RecalculateView()
	{
		Math::quat orientation = Math::quat(m_Orientation);
		m_ViewMatrix = glm::translate(Math::mat4(1.0f), m_Position) * glm::toMat4(orientation);
		m_ViewMatrix = glm::inverse(m_ViewMatrix);
	}
	void EditorOrthographicCamera::RecalculateProjection()
	{
		ViewportData& activeViewport = EngineService::GetActiveWindow().GetActiveViewport();
		float aspectRatio = (float)activeViewport.m_Width / (float)activeViewport.m_Height;
		float orthoLeft = -m_CameraSize * aspectRatio * 0.5f;
		float orthoRight = m_CameraSize * aspectRatio * 0.5f;
		float orthoBottom = -m_CameraSize * 0.5f;
		float orthoTop = m_CameraSize * 0.5f;
		m_Projection = glm::ortho(orthoLeft, orthoRight,
			orthoBottom, orthoTop, m_NearPlane, m_FarPlane);
	}

}
