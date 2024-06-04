#pragma once

#include "Kargono.h"

namespace Kargono
{
	class SceneHierarchyPanel
	{
	public:
		SceneHierarchyPanel();
	public:
		void OnEditorUIRender();
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);
	public:
		void SetSelectedEntity(Entity entity);
		void RefreshWidgetData();
	private:
		void DrawEntityNode(Entity entity);
		void DrawComponents(Entity entity);
	private:
		template<typename T>
		void DisplayAddComponentEntry(const std::string& entryName);
	private:
		std::string m_PanelName{"Scene Hierarchy"};
	};
}
