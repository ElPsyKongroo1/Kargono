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
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);
	public:
		void SetSelectedEntity(Scenes::Entity entity);
		void RefreshClassInstanceComponent();
		void RefreshTransformComponent();
	private:
		void DrawComponents(Scenes::Entity entity);
	private:
		std::string m_PanelName{"Scene Editor"};
	private:
		friend class PropertiesPanel;
	};
}
