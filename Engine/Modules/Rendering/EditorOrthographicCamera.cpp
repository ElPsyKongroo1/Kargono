#include "kgpch.h"

#include "Modules/Rendering/EditorOrthographicCamera.h"
#include "Modules/Input/InputService.h"
#include "Kargono/Core/KeyCodes.h"
#include "Kargono/Core/MouseCodes.h"
#include "Modules/Core/Engine.h"
#include "Modules/EditorUI/EditorUIInclude.h"

#include "API/Platform/GlfwAPI.h"


namespace Kargono::Rendering
{

	EditorOrthographicCamera::EditorOrthographicCamera(float size, float nearClip, float farClip) : m_CameraSize(size), m_NearPlane(nearClip), m_FarPlane(farClip), m_CameraSizeType(CameraSizeType::AspectRatio)
	{
		RecalculateProjection();
		RecalculateView();
	}
	EditorOrthographicCamera::EditorOrthographicCamera(Math::vec2 cameraSizeFixed, float nearClip, float farClip) : m_CameraSizeFixed(cameraSizeFixed), m_NearPlane(nearClip), m_FarPlane(farClip), m_CameraSizeType(CameraSizeType::Fixed), m_CameraSize(1.0f)
	{
		RecalculateProjection();
		RecalculateView();
	}

	EditorOrthographicCamera::EditorOrthographicCamera() : m_CameraSizeType(CameraSizeType::None)
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
			EngineService::GetActiveEngine().GetWindow().SetMouseCursorVisible(true);
		}
		return false;
	}

	bool EditorOrthographicCamera::OnKeyPressed(Events::KeyPressedEvent& e)
	{
		if (e.GetKeyCode() == Key::LeftAlt)
		{
			EngineService::GetActiveEngine().GetWindow().SetMouseCursorVisible(false);
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
		if (Input::InputService::IsKeyPressed(Key::LeftAlt) || Input::InputService::IsKeyPressed(Key::RightAlt))
		{

			if (m_CameraSizeType == CameraSizeType::AspectRatio)
			{
				float delta = e.GetYOffset() * 15.0f;
				if (m_CameraSize - delta <= 5.0f)
				{
					return false;
				}
				m_CameraSize -= delta;
				RecalculateProjection();
			}
			else if (m_CameraSizeType == CameraSizeType::Fixed)
			{
				// Get center point of camera before projection calculation
				Math::vec2 originalCenter
				{ 
					m_Position.x + ((m_CameraSizeFixed.x * m_CameraSize) / 2),
					m_Position.y + ((m_CameraSizeFixed.y * m_CameraSize) / 2)
				};

				// Calculate new camera size
				float delta = e.GetYOffset() * 0.1f;
				if (m_CameraSize - delta <= 0.01f)
				{
					return false;
				}
				m_CameraSize -= delta;
				
				// Get center point of camera after projection calculation
				Math::vec2 newCenter
				{
					m_Position.x + ((m_CameraSizeFixed.x * m_CameraSize) / 2),
					m_Position.y + ((m_CameraSizeFixed.y * m_CameraSize) / 2)
				};

				m_Position.x = m_Position.x - (newCenter.x - originalCenter.x);
				m_Position.y = m_Position.y - (newCenter.y - originalCenter.y);

				RecalculateProjection();
				RecalculateView();

			}
			else
			{
				KG_ERROR("Camera size type not set for EditorOrthographicCamera");
			}
			
		}

		return false;
	}
	void EditorOrthographicCamera::RecalculateView()
	{
		Math::quat orientation = Math::quat(m_Rotation);
		m_ViewMatrix = glm::translate(Math::mat4(1.0f), m_Position) * glm::toMat4(orientation);
		m_ViewMatrix = glm::inverse(m_ViewMatrix);
	}
	void EditorOrthographicCamera::RecalculateProjection()
	{
		ViewportData& activeViewport = EngineService::GetActiveEngine().GetWindow().GetActiveViewport();
		if (m_CameraSizeType == CameraSizeType::AspectRatio)
		{
			float aspectRatio = (float)activeViewport.m_Width / (float)activeViewport.m_Height;
			float orthoLeft = -m_CameraSize * aspectRatio * 0.5f;
			float orthoRight = m_CameraSize * aspectRatio * 0.5f;
			float orthoBottom = -m_CameraSize * 0.5f;
			float orthoTop = m_CameraSize * 0.5f;
			m_Projection = glm::ortho(orthoLeft, orthoRight,
				orthoBottom, orthoTop, m_NearPlane, m_FarPlane);
		}
		else if (m_CameraSizeType == CameraSizeType::Fixed)
		{
			float orthoLeft = 0.0f;
			float orthoRight = m_CameraSizeFixed.x * m_CameraSize;
			float orthoBottom = 0.0f;
			float orthoTop = m_CameraSizeFixed.y * m_CameraSize;

			/*float orthoLeft = -m_CameraSizeFixed.x * m_CameraSize * 0.5f;
			float orthoRight = m_CameraSizeFixed.x * m_CameraSize * 0.5f;
			float orthoBottom = -m_CameraSizeFixed.y * m_CameraSize * 0.5f;
			float orthoTop = m_CameraSizeFixed.y * m_CameraSize * 0.5f;*/
			m_Projection = glm::ortho(orthoLeft, orthoRight,
				orthoBottom, orthoTop, m_NearPlane, m_FarPlane);
		}
	}

}
