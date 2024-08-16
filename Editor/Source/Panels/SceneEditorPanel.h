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
		void SetSelectedEntity(Scenes::Entity entity);
		void RefreshClassInstanceComponent();
		void RefreshTransformComponent();
	public:
		void SetDisplayedComponent(Scenes::ComponentType type)
		{
			m_DisplayedComponent = type;
		}
	private:
		void DrawAllComponents(Scenes::Entity entity);
		void DrawSingleComponent(Scenes::Entity entity);
	private:
		void DrawTagComponent(Scenes::Entity entity);
		void DrawTransformComponent(Scenes::Entity entity);
		void DrawClassInstanceComponent(Scenes::Entity entity);
		void DrawRigidbody2DComponent(Scenes::Entity entity);
		void DrawBoxCollider2DComponent(Scenes::Entity entity);
		void DrawCircleCollider2DComponent(Scenes::Entity entity);
		void DrawCameraComponent(Scenes::Entity entity);
		void DrawShapeComponent(Scenes::Entity entity);
	private:
		std::string m_PanelName{"Scene Editor"};
		Scenes::ComponentType m_DisplayedComponent{Scenes::ComponentType::None };
	private:
		friend class PropertiesPanel;
	};
}
