#pragma once

#include "Kargono.h"

namespace Kargono::Panels
{
	class SceneEditorPanel
	{
	public:
		SceneEditorPanel();
	public:
		void OnEditorUIRender();
		bool OnSceneEvent(Events::Event* event);
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);
	public:
		void SetSelectedEntity(ECS::Entity entity);
		void RefreshClassInstanceComponent();
		void RefreshTransformComponent();
	public:
		void SetDisplayedComponent(ECS::ComponentType type)
		{
			m_DisplayedComponent = type;
		}
	private:
		void DrawAllComponents(ECS::Entity entity);
		void DrawSingleComponent(ECS::Entity entity);
	private:
		void DrawTagComponent(ECS::Entity entity);
		void DrawTransformComponent(ECS::Entity entity);
		void DrawClassInstanceComponent(ECS::Entity entity);
		void DrawRigidbody2DComponent(ECS::Entity entity);
		void DrawBoxCollider2DComponent(ECS::Entity entity);
		void DrawCircleCollider2DComponent(ECS::Entity entity);
		void DrawCameraComponent(ECS::Entity entity);
		void DrawShapeComponent(ECS::Entity entity);
	private:
		std::string m_PanelName{"Scene Editor"};
		ECS::ComponentType m_DisplayedComponent{ECS::ComponentType::None };
	private:
		friend class PropertiesPanel;
	};
}
