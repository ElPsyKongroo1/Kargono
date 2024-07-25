#pragma once

#include "Kargono.h"

namespace Kargono::Panels
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel();
	public:
		void OnEditorUIRender();
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);
	public:
		void SetSelectedEntity(Scenes::Entity entity);
		void RefreshClassInstanceComponent();
		void RefreshTransformComponent();
	private:
		void DrawEntityNode(Scenes::Entity entity);
		void DrawComponents(Scenes::Entity entity);
	private:
		std::string m_PanelName{"Scene Hierarchy"};
	private:
		friend class PropertiesPanel;
	};
}
