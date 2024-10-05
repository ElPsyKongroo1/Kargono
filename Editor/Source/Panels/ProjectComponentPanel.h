#pragma once
#include "Kargono.h"

namespace Kargono::Panels
{
	class ProjectComponentPanel
	{
	public:
		ProjectComponentPanel();

		void OnEditorUIRender();
	private:
		void InitializeOpeningPanel();
	public:
		bool OnKeyPressedEditor(Events::KeyPressedEvent event);

	private:
		void OpenComponent();
		void CreateComponent();
		void RefreshData();

	private:
		std::string m_PanelName{ "Project Component" };
		Ref<ECS::ProjectComponent> m_EditorProjectComponent { nullptr };
		Assets::AssetHandle m_EditorProjectComponentHandle { Assets::EmptyHandle };

	private:
		// Opening Panel Widgets
		EditorUI::GenericPopupSpec m_CreateComponentPopup {};
		EditorUI::SelectOptionSpec m_OpenComponentPopup {};
		EditorUI::EditTextSpec m_SelectComponentName {};
		// Header
		EditorUI::PanelHeaderSpec m_TagHeader {};
		EditorUI::GenericPopupSpec m_DeleteComponentWarning {};
		EditorUI::GenericPopupSpec m_CloseComponentWarning {};
	};
}


