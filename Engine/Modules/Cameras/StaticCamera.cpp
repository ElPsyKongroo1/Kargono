#include "kgpch.h"

#include "Modules/Cameras/StaticCamera.h"
#include "Modules/Core/Engine.h"

namespace Kargono::Cameras
{
	StaticCamera::StaticCamera()
	{
		RecalculateProjection();
	}
	void StaticCamera::SetOrthographic(float size, float nearClip, float farClip)
	{
		m_ProjectionType = ProjectionType::Orthographic;
		m_OrthographicSize = size;
		m_OrthographicNear = nearClip;
		m_OrthographicFar = farClip;

		RecalculateProjection();

	}
	void StaticCamera::SetPerspective(float verticalFOV, float nearClip, float farClip)
	{
		m_ProjectionType = ProjectionType::Perspective;
		m_PerspectiveFOV = verticalFOV;
		m_PerspectiveNear = nearClip;
		m_PerspectiveFar = farClip;

		RecalculateProjection();
	}
	void StaticCamera::OnViewportResize()
	{
		RecalculateProjection();
	}
	void StaticCamera::RecalculateProjection()
	{
		ViewportData& activeViewport = EngineService::GetActiveEngine().GetWindow().GetActiveViewport();
		float aspectRatio = (float)activeViewport.m_Width / (float)activeViewport.m_Height;

		if (m_ProjectionType == ProjectionType::Perspective)
		{
			m_CameraProjection.SetProjection(glm::perspective(m_PerspectiveFOV, 
				aspectRatio, m_PerspectiveNear, m_PerspectiveFar));
		}
		else
		{
			float orthoLeft = -m_OrthographicSize * aspectRatio * 0.5f;
			float orthoRight = m_OrthographicSize * aspectRatio * 0.5f;

			float orthoBottom = -m_OrthographicSize * 0.5f;
			float orthoTop = m_OrthographicSize * 0.5f;

			m_CameraProjection.SetProjection(glm::ortho(orthoLeft, orthoRight,
				orthoBottom, orthoTop, m_OrthographicNear, m_OrthographicFar));
		}
	}
}
