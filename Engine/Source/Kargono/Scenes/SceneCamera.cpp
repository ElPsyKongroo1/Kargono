#include "kgpch.h"

#include "Kargono/Scenes/SceneCamera.h"

#include "Kargono/Core/Engine.h"

namespace Kargono::Scenes
{
	SceneCamera::SceneCamera()
	{
		RecalculateProjection();
	}
	void SceneCamera::SetOrthographic(float size, float nearClip, float farClip)
	{
		m_ProjectionType = ProjectionType::Orthographic;
		m_OrthographicSize = size;
		m_OrthographicNear = nearClip;
		m_OrthographicFar = farClip;

		RecalculateProjection();

	}
	void SceneCamera::SetPerspective(float verticalFOV, float nearClip, float farClip)
	{
		m_ProjectionType = ProjectionType::Perspective;
		m_PerspectiveFOV = verticalFOV;
		m_PerspectiveNear = nearClip;
		m_PerspectiveFar = farClip;

		RecalculateProjection();
	}
	void SceneCamera::OnViewportResize()
	{
		RecalculateProjection();
	}
	void SceneCamera::RecalculateProjection()
	{
		ViewportData& activeViewport = EngineService::GetActiveWindow().GetActiveViewport();
		float aspectRatio = (float)activeViewport.m_Width / (float)activeViewport.m_Height;

		if (m_ProjectionType == ProjectionType::Perspective)
		{
			m_Projection = glm::perspective(m_PerspectiveFOV, aspectRatio, m_PerspectiveNear, m_PerspectiveFar);
		}
		else
		{
			float orthoLeft = -m_OrthographicSize * aspectRatio * 0.5f;
			float orthoRight = m_OrthographicSize * aspectRatio * 0.5f;

			float orthoBottom = -m_OrthographicSize * 0.5f;
			float orthoTop = m_OrthographicSize * 0.5f;


			m_Projection = glm::ortho(orthoLeft, orthoRight,
				orthoBottom, orthoTop, m_OrthographicNear, m_OrthographicFar);
		}
	}
}
